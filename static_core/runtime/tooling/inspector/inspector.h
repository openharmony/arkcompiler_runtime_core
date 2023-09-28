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

#ifndef PANDA_TOOLING_INSPECTOR_INSPECTOR_H
#define PANDA_TOOLING_INSPECTOR_INSPECTOR_H

#include "debug_info_cache.h"
#include "inspector_server.h"
#include "debuggable_thread.h"
#include "types/numeric_id.h"

#include "tooling/debug_interface.h"
#include "tooling/inspector/object_repository.h"
#include "tooling/inspector/types/pause_on_exceptions_state.h"
#include "tooling/inspector/types/property_descriptor.h"
#include "tooling/inspector/types/remote_object.h"
#include "tooling/pt_thread.h"

#include <array>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <string_view>
#include <thread>
#include <vector>

namespace panda::tooling {
class DebugInterface;

namespace inspector {
// NOLINTNEXTLINE(fuchsia-virtual-inheritance)
class Server;

class Inspector : public PtHooks {
public:
    Inspector(Server &server, DebugInterface &debugger, bool break_on_start);
    ~Inspector() override;

    NO_COPY_SEMANTIC(Inspector);
    NO_MOVE_SEMANTIC(Inspector);

    void ConsoleCall(PtThread thread, ConsoleCallType type, uint64_t timestamp,
                     const PandaVector<TypedValue> &arguments) override;
    void Exception(PtThread thread, Method *method, const PtLocation &location, ObjectHeader *exception,
                   Method *catch_method, const PtLocation &catch_location) override;
    void FramePop(PtThread thread, Method *method, bool was_popped_by_exception) override;
    void MethodEntry(PtThread thread, Method *method) override;
    void LoadModule(std::string_view file_name) override;
    void SingleStep(PtThread thread, Method *method, const PtLocation &location) override;
    void ThreadStart(PtThread thread) override;
    void ThreadEnd(PtThread thread) override;

private:
    void RuntimeEnable(PtThread thread);

    void RunIfWaitingForDebugger(PtThread thread);

    void Pause(PtThread thread);
    void Continue(PtThread thread);

    void SetBreakpointsActive(PtThread thread, bool active);
    std::set<size_t> GetPossibleBreakpoints(std::string_view source_file, size_t start_line, size_t end_line,
                                            bool restrict_to_function);
    std::optional<BreakpointId> SetBreakpoint(PtThread thread,
                                              const std::function<bool(std::string_view)> &source_files_filter,
                                              size_t line_number, std::set<std::string_view> &source_files);
    void RemoveBreakpoint(PtThread thread, BreakpointId breakpoint_id);

    void SetPauseOnExceptions(PtThread thread, PauseOnExceptionsState state);

    void StepInto(PtThread thread);
    void StepOver(PtThread thread);
    void StepOut(PtThread thread);
    void ContinueToLocation(PtThread thread, std::string_view source_file, size_t line_number);

    void RestartFrame(PtThread thread, FrameId frame_id);

    std::vector<PropertyDescriptor> GetProperties(PtThread thread, RemoteObjectId object_id, bool generate_preview);
    std::string GetSourceCode(std::string_view source_file);

private:
    bool break_on_start_;

    os::memory::RWLock debugger_events_lock_;
    bool connecting_ {false};  // Should be accessed only from the server thread

    InspectorServer inspector_server_;  // NOLINT(misc-non-private-member-variables-in-classes)
    DebugInterface &debugger_;
    DebugInfoCache debug_info_cache_;
    std::map<PtThread, DebuggableThread> threads_;

    std::thread server_thread_;
};
}  // namespace inspector
}  // namespace panda::tooling

#endif  // PANDA_TOOLING_INSPECTOR_INSPECTOR_H
