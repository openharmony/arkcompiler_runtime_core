/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include <deque>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "macros.h"
#include "plugins.h"
#include "runtime.h"
#include "utils/logger.h"

#include "error.h"
#include "evaluation/base64.h"
#include "types/remote_object.h"
#include "types/scope.h"

using namespace std::placeholders;  // NOLINT(google-build-using-namespace)

namespace ark::tooling::inspector {
static void LogDebuggerNotPaused(std::string_view methodName)
{
    LOG(WARNING, DEBUGGER) << "Inspector method '" << methodName << "' must be called on pause";
}

Inspector::Inspector(Server &server, DebugInterface &debugger, bool breakOnStart)
    : breakOnStart_(breakOnStart), inspectorServer_(server), debugger_(debugger)
{
    if (!HandleError(debugger_.RegisterHooks(this))) {
        return;
    }

    // acquire lock to later release it either in `OnOpen` or `OnFail` callbacks
    inspectorServer_.OnValidate([this]() NO_THREAD_SAFETY_ANALYSIS {
        ASSERT(!connecting_);  // NOLINT(bugprone-lambda-function-name)
        debuggerEventsLock_.WriteLock();
        connecting_ = true;
    });
    inspectorServer_.OnOpen([this]() NO_THREAD_SAFETY_ANALYSIS {
        ASSERT(connecting_);  // NOLINT(bugprone-lambda-function-name)

        for (auto &[_, dbgThread] : threads_) {
            (void)_;
            dbgThread.Reset();
        }

        connecting_ = false;
        debuggerEventsLock_.Unlock();
    });
    inspectorServer_.OnFail([this]() NO_THREAD_SAFETY_ANALYSIS {
        if (connecting_) {
            connecting_ = false;
            debuggerEventsLock_.Unlock();
        }
    });

    // NOLINTBEGIN(modernize-avoid-bind)
    inspectorServer_.OnCallDebuggerContinueToLocation(std::bind(&Inspector::ContinueToLocation, this, _1, _2, _3));
    inspectorServer_.OnCallDebuggerGetPossibleBreakpoints(
        std::bind(&Inspector::GetPossibleBreakpoints, this, _1, _2, _3, _4));
    inspectorServer_.OnCallDebuggerGetScriptSource(std::bind(&Inspector::GetSourceCode, this, _1));
    inspectorServer_.OnCallDebuggerPause(std::bind(&Inspector::Pause, this, _1));
    inspectorServer_.OnCallDebuggerRemoveBreakpoint(std::bind(&Inspector::RemoveBreakpoint, this, _1, _2));
    inspectorServer_.OnCallDebuggerRestartFrame(std::bind(&Inspector::RestartFrame, this, _1, _2));
    inspectorServer_.OnCallDebuggerResume(std::bind(&Inspector::Continue, this, _1));
    inspectorServer_.OnCallDebuggerSetBreakpoint(std::bind(&Inspector::SetBreakpoint, this, _1, _2, _3, _4, _5));
    inspectorServer_.OnCallDebuggerSetBreakpointByUrl(std::bind(&Inspector::SetBreakpoint, this, _1, _2, _3, _4, _5));
    inspectorServer_.OnCallDebuggerSetBreakpointsActive(std::bind(&Inspector::SetBreakpointsActive, this, _1, _2));
    inspectorServer_.OnCallDebuggerSetPauseOnExceptions(std::bind(&Inspector::SetPauseOnExceptions, this, _1, _2));
    inspectorServer_.OnCallDebuggerStepInto(std::bind(&Inspector::StepInto, this, _1));
    inspectorServer_.OnCallDebuggerStepOut(std::bind(&Inspector::StepOut, this, _1));
    inspectorServer_.OnCallDebuggerStepOver(std::bind(&Inspector::StepOver, this, _1));
    inspectorServer_.OnCallRuntimeEnable(std::bind(&Inspector::RuntimeEnable, this, _1));
    inspectorServer_.OnCallRuntimeGetProperties(std::bind(&Inspector::GetProperties, this, _1, _2, _3));
    inspectorServer_.OnCallRuntimeRunIfWaitingForDebugger(std::bind(&Inspector::RunIfWaitingForDebugger, this, _1));
    inspectorServer_.OnCallRuntimeEvaluate(std::bind(&Inspector::Evaluate, this, _1, _2));
    // NOLINTEND(modernize-avoid-bind)

    serverThread_ = std::thread(&InspectorServer::Run, &inspectorServer_);
    os::thread::SetThreadName(serverThread_.native_handle(), "InspectorServer");
}

Inspector::~Inspector()
{
    // Current implementation destroys `Inspector` after server connection is closed,
    // hence no need to notify client
    inspectorServer_.Kill();
    serverThread_.join();
    HandleError(debugger_.UnregisterHooks());
}

void Inspector::ConsoleCall(PtThread thread, ConsoleCallType type, uint64_t timestamp,
                            const PandaVector<TypedValue> &arguments)
{
    os::memory::ReadLockHolder lock(debuggerEventsLock_);

    auto *debuggableThread = GetDebuggableThread(thread);
    ASSERT(debuggableThread != nullptr);
    inspectorServer_.CallRuntimeConsoleApiCalled(thread, type, timestamp, debuggableThread->OnConsoleCall(arguments));
}

// CC-OFFNXT(G.FUN.01-CPP) Decreasing the number of arguments will decrease the clarity of the code.
void Inspector::Exception(PtThread thread, Method * /* method */, const PtLocation & /* location */,
                          ObjectHeader * /* exception */, Method * /* catch_method */, const PtLocation &catchLocation)
{
    os::memory::ReadLockHolder lock(debuggerEventsLock_);

    auto *debuggableThread = GetDebuggableThread(thread);
    ASSERT(debuggableThread != nullptr);
    debuggableThread->OnException(catchLocation.GetBytecodeOffset() == panda_file::INVALID_OFFSET);
}

void Inspector::FramePop(PtThread thread, Method * /* method */, bool /* was_popped_by_exception */)
{
    os::memory::ReadLockHolder lock(debuggerEventsLock_);

    auto *debuggableThread = GetDebuggableThread(thread);
    ASSERT(debuggableThread != nullptr);
    debuggableThread->OnFramePop();
}

void Inspector::MethodEntry(PtThread thread, Method * /* method */)
{
    os::memory::ReadLockHolder lock(debuggerEventsLock_);

    auto *debuggableThread = GetDebuggableThread(thread);
    ASSERT(debuggableThread != nullptr);
    if (debuggableThread->OnMethodEntry()) {
        HandleError(debugger_.NotifyFramePop(thread, 0));
    }
}

void Inspector::LoadModule(std::string_view fileName)
{
    os::memory::ReadLockHolder lock(debuggerEventsLock_);

    Runtime::GetCurrent()->GetClassLinker()->EnumeratePandaFiles(
        [this, fileName](auto &file) {
            if (file.GetFilename() == fileName) {
                debugInfoCache_.AddPandaFile(file);
            }

            return true;
        },
        !fileName.empty());
}

void Inspector::SingleStep(PtThread thread, Method * /* method */, const PtLocation &location)
{
    os::memory::ReadLockHolder lock(debuggerEventsLock_);

    auto *debuggableThread = GetDebuggableThread(thread);
    ASSERT(debuggableThread != nullptr);
    debuggableThread->OnSingleStep(location);
}

void Inspector::ThreadStart(PtThread thread)
{
    os::memory::ReadLockHolder lock(debuggerEventsLock_);

    if (thread != PtThread::NONE) {
        inspectorServer_.CallTargetAttachedToTarget(thread);
    }

    // NOLINTBEGIN(modernize-avoid-bind)
    auto callbacks = DebuggableThread::SuspensionCallbacks {
        [](auto &, auto &, auto) {},
        std::bind(&Inspector::DebuggableThreadPostSuspend, this, thread, _1, _2, _3),
        [this]() NO_THREAD_SAFETY_ANALYSIS { debuggerEventsLock_.Unlock(); },
        [this]() NO_THREAD_SAFETY_ANALYSIS { debuggerEventsLock_.ReadLock(); },
        []() {},
        [this, thread]() { inspectorServer_.CallDebuggerResumed(thread); }};
    // NOLINTEND(modernize-avoid-bind)
    auto [it, inserted] =
        threads_.emplace(std::piecewise_construct, std::forward_as_tuple(thread),
                         std::forward_as_tuple(thread.GetManagedThread(), &debugger_, std::move(callbacks)));
    (void)inserted;
    ASSERT(inserted);

    if (breakOnStart_) {
        it->second.BreakOnStart();
    }
}

void Inspector::ThreadEnd(PtThread thread)
{
    os::memory::ReadLockHolder lock(debuggerEventsLock_);

    if (thread != PtThread::NONE) {
        inspectorServer_.CallTargetDetachedFromTarget(thread);
    }

    [[maybe_unused]] auto erased = threads_.erase(thread);
    ASSERT(erased == 1);
}

void Inspector::VmDeath()
{
    os::memory::WriteLockHolder lock(vmDeathLock_);

    ASSERT(!isVmDead_);
    isVmDead_ = true;

    NotifyExecutionEnded();
}

void Inspector::RuntimeEnable(PtThread thread)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    inspectorServer_.CallRuntimeExecutionContextCreated(thread);
}

void Inspector::RunIfWaitingForDebugger(PtThread thread)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        debuggableThread->Touch();
    }
}

void Inspector::Pause(PtThread thread)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        debuggableThread->Pause();
    }
}

void Inspector::Continue(PtThread thread)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        debuggableThread->Continue();
    }
}

void Inspector::SetBreakpointsActive(PtThread thread, bool active)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        debuggableThread->SetBreakpointsActive(active);
    }
}

std::set<size_t> Inspector::GetPossibleBreakpoints(std::string_view sourceFile, size_t startLine, size_t endLine,
                                                   bool restrictToFunction)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return {};
    }

    return debugInfoCache_.GetValidLineNumbers(sourceFile, startLine, endLine, restrictToFunction);
}

std::optional<BreakpointId> Inspector::SetBreakpoint(PtThread thread,
                                                     const std::function<bool(std::string_view)> &sourceFilesFilter,
                                                     size_t lineNumber, std::set<std::string_view> &sourceFiles,
                                                     const std::string *condition)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return {};
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread == nullptr) {
        return {};
    }

    auto locations = debugInfoCache_.GetBreakpointLocations(sourceFilesFilter, lineNumber, sourceFiles);
    std::string optBytecode;
    if (condition != nullptr) {
        if (condition->empty()) {
            // Some debugger clients send empty condition by default
            condition = nullptr;
        } else {
            Base64Decoder::Decode(*condition, optBytecode);
            condition = &optBytecode;
        }
    }
    return debuggableThread->SetBreakpoint(locations, condition);
}

void Inspector::RemoveBreakpoint(PtThread thread, BreakpointId id)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        debuggableThread->RemoveBreakpoint(id);
    }
}

void Inspector::SetPauseOnExceptions(PtThread thread, PauseOnExceptionsState state)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        debuggableThread->SetPauseOnExceptions(state);
    }
}

void Inspector::StepInto(PtThread thread)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        if (UNLIKELY(!debuggableThread->IsPaused())) {
            LogDebuggerNotPaused("stepInto");
            return;
        }

        auto frame = debugger_.GetCurrentFrame(thread);
        if (!frame) {
            HandleError(frame.Error());
            return;
        }

        debuggableThread->StepInto(debugInfoCache_.GetCurrentLineLocations(*frame.Value()));
    }
}

void Inspector::StepOver(PtThread thread)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        if (UNLIKELY(!debuggableThread->IsPaused())) {
            LogDebuggerNotPaused("stepOver");
            return;
        }

        auto frame = debugger_.GetCurrentFrame(thread);
        if (!frame) {
            HandleError(frame.Error());
            return;
        }

        debuggableThread->StepOver(debugInfoCache_.GetCurrentLineLocations(*frame.Value()));
    }
}

void Inspector::StepOut(PtThread thread)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        if (UNLIKELY(!debuggableThread->IsPaused())) {
            LogDebuggerNotPaused("stepOut");
            return;
        }

        HandleError(debugger_.NotifyFramePop(thread, 0));
        debuggableThread->StepOut();
    }
}

void Inspector::ContinueToLocation(PtThread thread, std::string_view sourceFile, size_t lineNumber)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        if (UNLIKELY(!debuggableThread->IsPaused())) {
            LogDebuggerNotPaused("continueToLocation");
            return;
        }

        debuggableThread->ContinueTo(debugInfoCache_.GetContinueToLocations(sourceFile, lineNumber));
    }
}

void Inspector::RestartFrame(PtThread thread, FrameId frameId)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        if (UNLIKELY(!debuggableThread->IsPaused())) {
            LogDebuggerNotPaused("restartFrame");
            return;
        }

        if (auto error = debugger_.RestartFrame(thread, frameId)) {
            HandleError(*error);
            return;
        }

        debuggableThread->StepInto({});
    }
}

std::vector<PropertyDescriptor> Inspector::GetProperties(PtThread thread, RemoteObjectId objectId, bool generatePreview)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return {};
    }

    std::optional<std::vector<PropertyDescriptor>> properties;

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        debuggableThread->RequestToObjectRepository([objectId, generatePreview, &properties](auto &objectRepository) {
            properties = objectRepository.GetProperties(objectId, generatePreview);
        });
    }

    if (!properties) {
        LOG(INFO, DEBUGGER) << "Failed to resolve object id: " << objectId;
        return {};
    }

    return *properties;
}

std::string Inspector::GetSourceCode(std::string_view sourceFile)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return {};
    }

    return debugInfoCache_.GetSourceCode(sourceFile);
}

void Inspector::DebuggableThreadPostSuspend(PtThread thread, ObjectRepository &objectRepository,
                                            const std::vector<BreakpointId> &hitBreakpoints, ObjectHeader *exception)
{
    auto exceptionRemoteObject = exception != nullptr ? objectRepository.CreateObject(TypedValue::Reference(exception))
                                                      : std::optional<RemoteObject>();

    inspectorServer_.CallDebuggerPaused(
        thread, hitBreakpoints, exceptionRemoteObject, [this, thread, &objectRepository](auto &handler) {
            FrameId frameId = 0;
            HandleError(debugger_.EnumerateFrames(thread, [this, &objectRepository, &handler,
                                                           &frameId](const PtFrame &frame) {
                std::string_view sourceFile;
                std::string_view methodName;
                size_t lineNumber;
                debugInfoCache_.GetSourceLocation(frame, sourceFile, methodName, lineNumber);

                std::optional<RemoteObject> objThis;
                auto frameObject = objectRepository.CreateFrameObject(frame, debugInfoCache_.GetLocals(frame), objThis);
                auto scopeChain = std::vector {Scope(Scope::Type::LOCAL, std::move(frameObject)),
                                               Scope(Scope::Type::GLOBAL, objectRepository.CreateGlobalObject())};

                handler(frameId++, methodName, sourceFile, lineNumber, scopeChain, objThis);

                return true;
            }));
        });
}

void Inspector::NotifyExecutionEnded()
{
    inspectorServer_.CallRuntimeExecutionContextsCleared();
}

InspectorServer::EvaluationResult Inspector::Evaluate(PtThread thread, const std::string &bytecodeBase64)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return {};
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread == nullptr) {
        return {};
    }

    if (UNLIKELY(!debuggableThread->IsPaused())) {
        LogDebuggerNotPaused("evaluate");
        return {};
    }

    std::string bytecode;
    Base64Decoder::Decode(bytecodeBase64, bytecode);
    auto optResult = debuggableThread->EvaluateExpression(0, bytecode);
    if (!optResult) {
        return {};
    }
    auto optExceptionDetails = (optResult->second) ? CreateExceptionDetails(thread, std::move(*optResult->second))
                                                   : std::optional<ExceptionDetails>();
    return std::make_pair(optResult->first, optExceptionDetails);
}

std::optional<ExceptionDetails> Inspector::CreateExceptionDetails(PtThread thread, RemoteObject &&exception)
{
    auto frame = debugger_.GetCurrentFrame(thread);
    if (!frame) {
        HandleError(frame.Error());
        return {};
    }

    std::string_view sourceFile;
    std::string_view methodName;
    size_t lineNumber;
    debugInfoCache_.GetSourceLocation(*frame.Value(), sourceFile, methodName, lineNumber);

    ExceptionDetails exceptionDetails(GetNewExceptionId(), "", lineNumber, 0);
    return exceptionDetails.SetUrl(sourceFile).SetExceptionObject(std::move(exception));
}

size_t Inspector::GetNewExceptionId()
{
    // Atomic with relaxed order reason: data race on concurrent exceptions happening in conditional breakpoints.
    return currentExceptionId_.fetch_add(1, std::memory_order_relaxed);
}

DebuggableThread *Inspector::GetDebuggableThread(PtThread thread)
{
    auto it = threads_.find(thread);
    return it != threads_.end() ? &it->second : nullptr;
}
}  // namespace ark::tooling::inspector
