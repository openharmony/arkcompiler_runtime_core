/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "inspector.h"

#include "error.h"

#include "macros.h"
#include "plugins.h"
#include "runtime.h"
#include "tooling/inspector/types/remote_object.h"
#include "tooling/inspector/types/scope.h"
#include "utils/logger.h"

#include <deque>
#include <functional>
#include <string>
#include <utility>
#include <vector>

using namespace std::placeholders;  // NOLINT(google-build-using-namespace)

namespace panda::tooling::inspector {
Inspector::Inspector(Server &server, DebugInterface &debugger, bool break_on_start)
    : break_on_start_(break_on_start), inspector_server_(server), debugger_(debugger)
{
    if (!HandleError(debugger_.RegisterHooks(this))) {
        return;
    }

    inspector_server_.OnValidate([this]() NO_THREAD_SAFETY_ANALYSIS {
        ASSERT(!connecting_);  // NOLINT(bugprone-lambda-function-name)
        debugger_events_lock_.WriteLock();
        connecting_ = true;
    });
    inspector_server_.OnOpen([this]() NO_THREAD_SAFETY_ANALYSIS {
        ASSERT(connecting_);  // NOLINT(bugprone-lambda-function-name)

        for (auto &[thread, dbg_thread] : threads_) {
            (void)thread;
            dbg_thread.Reset();
        }

        connecting_ = false;
        debugger_events_lock_.Unlock();
    });
    inspector_server_.OnFail([this]() NO_THREAD_SAFETY_ANALYSIS {
        if (connecting_) {
            connecting_ = false;
            debugger_events_lock_.Unlock();
        }
    });

    // NOLINTBEGIN(modernize-avoid-bind)
    inspector_server_.OnCallDebuggerContinueToLocation(std::bind(&Inspector::ContinueToLocation, this, _1, _2, _3));
    inspector_server_.OnCallDebuggerGetPossibleBreakpoints(
        std::bind(&Inspector::GetPossibleBreakpoints, this, _1, _2, _3, _4));
    inspector_server_.OnCallDebuggerGetScriptSource(std::bind(&Inspector::GetSourceCode, this, _1));
    inspector_server_.OnCallDebuggerPause(std::bind(&Inspector::Pause, this, _1));
    inspector_server_.OnCallDebuggerRemoveBreakpoint(std::bind(&Inspector::RemoveBreakpoint, this, _1, _2));
    inspector_server_.OnCallDebuggerRestartFrame(std::bind(&Inspector::RestartFrame, this, _1, _2));
    inspector_server_.OnCallDebuggerResume(std::bind(&Inspector::Continue, this, _1));
    inspector_server_.OnCallDebuggerSetBreakpoint(std::bind(&Inspector::SetBreakpoint, this, _1, _2, _3, _4));
    inspector_server_.OnCallDebuggerSetBreakpointByUrl(std::bind(&Inspector::SetBreakpoint, this, _1, _2, _3, _4));
    inspector_server_.OnCallDebuggerSetBreakpointsActive(std::bind(&Inspector::SetBreakpointsActive, this, _1, _2));
    inspector_server_.OnCallDebuggerSetPauseOnExceptions(std::bind(&Inspector::SetPauseOnExceptions, this, _1, _2));
    inspector_server_.OnCallDebuggerStepInto(std::bind(&Inspector::StepInto, this, _1));
    inspector_server_.OnCallDebuggerStepOut(std::bind(&Inspector::StepOut, this, _1));
    inspector_server_.OnCallDebuggerStepOver(std::bind(&Inspector::StepOver, this, _1));
    inspector_server_.OnCallRuntimeEnable(std::bind(&Inspector::RuntimeEnable, this, _1));
    inspector_server_.OnCallRuntimeGetProperties(std::bind(&Inspector::GetProperties, this, _1, _2, _3));
    inspector_server_.OnCallRuntimeRunIfWaitingForDebugger(std::bind(&Inspector::RunIfWaitingForDebugger, this, _1));
    // NOLINTEND(modernize-avoid-bind)

    server_thread_ = std::thread(&InspectorServer::Run, &inspector_server_);
    os::thread::SetThreadName(server_thread_.native_handle(), "InspectorServer");
}

Inspector::~Inspector()
{
    inspector_server_.Kill();
    server_thread_.join();
    HandleError(debugger_.UnregisterHooks());
}

void Inspector::ConsoleCall(PtThread thread, ConsoleCallType type, uint64_t timestamp,
                            const PandaVector<TypedValue> &arguments)
{
    os::memory::ReadLockHolder lock(debugger_events_lock_);

    auto it = threads_.find(thread);
    ASSERT(it != threads_.end());

    inspector_server_.CallRuntimeConsoleApiCalled(thread, type, timestamp, it->second.OnConsoleCall(arguments));
}

void Inspector::Exception(PtThread thread, Method * /* method */, const PtLocation & /* location */,
                          ObjectHeader * /* exception */, Method * /* catch_method */, const PtLocation &catch_location)
{
    os::memory::ReadLockHolder lock(debugger_events_lock_);

    auto it = threads_.find(thread);
    ASSERT(it != threads_.end());
    it->second.OnException(catch_location.GetBytecodeOffset() == panda_file::INVALID_OFFSET);
}

void Inspector::FramePop(PtThread thread, Method * /* method */, bool /* was_popped_by_exception */)
{
    os::memory::ReadLockHolder lock(debugger_events_lock_);

    auto it = threads_.find(thread);
    ASSERT(it != threads_.end());
    it->second.OnFramePop();
}

void Inspector::MethodEntry(PtThread thread, Method * /* method */)
{
    os::memory::ReadLockHolder lock(debugger_events_lock_);

    auto it = threads_.find(thread);
    ASSERT(it != threads_.end());
    if (it->second.OnMethodEntry()) {
        HandleError(debugger_.NotifyFramePop(thread, 0));
    }
}

void Inspector::LoadModule(std::string_view file_name)
{
    os::memory::ReadLockHolder lock(debugger_events_lock_);

    Runtime::GetCurrent()->GetClassLinker()->EnumeratePandaFiles(
        [this, file_name](auto &file) {
            if (file.GetFilename() == file_name) {
                debug_info_cache_.AddPandaFile(file);
            }

            return true;
        },
        !file_name.empty());
}

void Inspector::SingleStep(PtThread thread, Method * /* method */, const PtLocation &location)
{
    os::memory::ReadLockHolder lock(debugger_events_lock_);

    auto it = threads_.find(thread);
    ASSERT(it != threads_.end());
    it->second.OnSingleStep(location);
}

void Inspector::ThreadStart(PtThread thread)
{
    os::memory::ReadLockHolder lock(debugger_events_lock_);

    if (thread != PtThread::NONE) {
        inspector_server_.CallTargetAttachedToTarget(thread);
    }

    auto [it, inserted] = threads_.emplace(
        std::piecewise_construct, std::forward_as_tuple(thread),
        std::forward_as_tuple(
            thread.GetManagedThread(), [](auto &, auto &, auto) {},
            [this, thread](auto &object_repository, auto &hit_breakpoints, auto exception) {
                auto exception_remote_object = exception != nullptr
                                                   ? object_repository.CreateObject(TypedValue::Reference(exception))
                                                   : std::optional<RemoteObject>();

                inspector_server_.CallDebuggerPaused(
                    thread, hit_breakpoints, exception_remote_object,
                    [this, thread, &object_repository](auto &handler) {
                        FrameId frame_id = 0;
                        HandleError(debugger_.EnumerateFrames(thread, [this, &object_repository, &handler,
                                                                       &frame_id](const PtFrame &frame) {
                            std::string_view source_file;
                            std::string_view method_name;
                            size_t line_number;
                            debug_info_cache_.GetSourceLocation(frame, source_file, method_name, line_number);

                            auto scope_chain = std::vector {
                                Scope(Scope::Type::LOCAL,
                                      object_repository.CreateFrameObject(frame, debug_info_cache_.GetLocals(frame))),
                                Scope(Scope::Type::GLOBAL, object_repository.CreateGlobalObject())};

                            handler(frame_id++, method_name, source_file, line_number, scope_chain);

                            return true;
                        }));
                    });
            },
            [this]() NO_THREAD_SAFETY_ANALYSIS { debugger_events_lock_.Unlock(); },
            [this]() NO_THREAD_SAFETY_ANALYSIS { debugger_events_lock_.ReadLock(); }, []() {},
            [this, thread]() { inspector_server_.CallDebuggerResumed(thread); }));
    (void)inserted;
    ASSERT(inserted);

    if (break_on_start_) {
        it->second.BreakOnStart();
    }
}

void Inspector::ThreadEnd(PtThread thread)
{
    os::memory::ReadLockHolder lock(debugger_events_lock_);

    if (thread != PtThread::NONE) {
        inspector_server_.CallTargetDetachedFromTarget(thread);
    }

    threads_.erase(thread);
}

void Inspector::RuntimeEnable(PtThread thread)
{
    inspector_server_.CallRuntimeExecutionContextCreated(thread);
}

void Inspector::RunIfWaitingForDebugger(PtThread thread)
{
    auto it = threads_.find(thread);
    if (it != threads_.end()) {
        it->second.Touch();
    }
}

void Inspector::Pause(PtThread thread)
{
    auto it = threads_.find(thread);
    if (it != threads_.end()) {
        it->second.Pause();
    }
}

void Inspector::Continue(PtThread thread)
{
    auto it = threads_.find(thread);
    if (it != threads_.end()) {
        it->second.Continue();
    }
}

void Inspector::SetBreakpointsActive(PtThread thread, bool active)
{
    auto it = threads_.find(thread);
    if (it != threads_.end()) {
        it->second.SetBreakpointsActive(active);
    }
}

std::set<size_t> Inspector::GetPossibleBreakpoints(std::string_view source_file, size_t start_line, size_t end_line,
                                                   bool restrict_to_function)
{
    return debug_info_cache_.GetValidLineNumbers(source_file, start_line, end_line, restrict_to_function);
}

std::optional<BreakpointId> Inspector::SetBreakpoint(PtThread thread,
                                                     const std::function<bool(std::string_view)> &source_files_filter,
                                                     size_t line_number, std::set<std::string_view> &source_files)
{
    if (auto it = threads_.find(thread); it != threads_.end()) {
        auto locations = debug_info_cache_.GetBreakpointLocations(source_files_filter, line_number, source_files);
        return it->second.SetBreakpoint(locations);
    }

    return {};
}

void Inspector::RemoveBreakpoint(PtThread thread, BreakpointId id)
{
    auto it = threads_.find(thread);
    if (it != threads_.end()) {
        it->second.RemoveBreakpoint(id);
    }
}

void Inspector::SetPauseOnExceptions(PtThread thread, PauseOnExceptionsState state)
{
    auto it = threads_.find(thread);
    if (it != threads_.end()) {
        it->second.SetPauseOnExceptions(state);
    }
}

void Inspector::StepInto(PtThread thread)
{
    auto it = threads_.find(thread);
    if (it != threads_.end()) {
        auto frame = debugger_.GetCurrentFrame(thread);
        if (!frame) {
            HandleError(frame.Error());
            return;
        }

        it->second.StepInto(debug_info_cache_.GetCurrentLineLocations(*frame.Value()));
    }
}

void Inspector::StepOver(PtThread thread)
{
    auto it = threads_.find(thread);
    if (it != threads_.end()) {
        auto frame = debugger_.GetCurrentFrame(thread);
        if (!frame) {
            HandleError(frame.Error());
            return;
        }

        it->second.StepOver(debug_info_cache_.GetCurrentLineLocations(*frame.Value()));
    }
}

void Inspector::StepOut(PtThread thread)
{
    auto it = threads_.find(thread);
    if (it != threads_.end()) {
        HandleError(debugger_.NotifyFramePop(thread, 0));
        it->second.StepOut();
    }
}

void Inspector::ContinueToLocation(PtThread thread, std::string_view source_file, size_t line_number)
{
    auto it = threads_.find(thread);
    if (it != threads_.end()) {
        it->second.ContinueTo(debug_info_cache_.GetContinueToLocations(source_file, line_number));
    }
}

void Inspector::RestartFrame(PtThread thread, FrameId frame_id)
{
    auto it = threads_.find(thread);
    if (it != threads_.end()) {
        if (auto error = debugger_.RestartFrame(thread, frame_id)) {
            HandleError(*error);
            return;
        }

        it->second.StepInto({});
    }
}

std::vector<PropertyDescriptor> Inspector::GetProperties(PtThread thread, RemoteObjectId object_id,
                                                         bool generate_preview)
{
    std::optional<std::vector<PropertyDescriptor>> properties;

    auto it = threads_.find(thread);
    if (it != threads_.end()) {
        it->second.RequestToObjectRepository([object_id, generate_preview, &properties](auto &object_repository) {
            properties = object_repository.GetProperties(object_id, generate_preview);
        });
    }

    if (!properties) {
        LOG(INFO, DEBUGGER) << "Failed to resolve object id: " << object_id;
        return {};
    }

    return *properties;
}

std::string Inspector::GetSourceCode(std::string_view source_file)
{
    return debug_info_cache_.GetSourceCode(source_file);
}
}  // namespace panda::tooling::inspector
