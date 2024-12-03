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

#ifndef PANDA_TOOLING_INSPECTOR_INSPECTOR_SERVER_H
#define PANDA_TOOLING_INSPECTOR_INSPECTOR_SERVER_H

#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <optional>
#include <set>
#include <string_view>
#include <vector>

#include "console_call_type.h"
#include "include/tooling/pt_thread.h"

#include "session_manager.h"
#include "source_manager.h"
#include "types/exception_details.h"
#include "types/location.h"
#include "types/numeric_id.h"
#include "types/pause_on_exceptions_state.h"
#include "types/property_descriptor.h"
#include "types/remote_object.h"
#include "types/scope.h"

namespace ark::tooling::inspector {
class Server;  // NOLINT(fuchsia-virtual-inheritance)

class InspectorServer final {
public:
    using SetBreakpointHandler = std::optional<BreakpointId>(PtThread, const std::function<bool(std::string_view)> &,
                                                             size_t, std::set<std::string_view> &, const std::string *);
    using FrameInfoHandler = std::function<void(FrameId, std::string_view, std::string_view, size_t,
                                                const std::vector<Scope> &, const std::optional<RemoteObject> &)>;
    using EvaluationResult = std::optional<std::pair<RemoteObject, std::optional<ExceptionDetails>>>;

public:
    explicit InspectorServer(Server &server);
    ~InspectorServer() = default;

    NO_COPY_SEMANTIC(InspectorServer);
    NO_MOVE_SEMANTIC(InspectorServer);

    void Kill();
    void Run();

    void OnValidate(std::function<void()> &&handler);
    void OnOpen(std::function<void()> &&handler);
    void OnFail(std::function<void()> &&handler);

    void CallDebuggerPaused(PtThread thread, const std::vector<BreakpointId> &hitBreakpoints,
                            const std::optional<RemoteObject> &exception,
                            const std::function<void(const FrameInfoHandler &)> &enumerateFrames);
    void CallDebuggerResumed(PtThread thread);
    void CallDebuggerScriptParsed(PtThread thread, ScriptId scriptId, std::string_view sourceFile);
    void CallRuntimeConsoleApiCalled(PtThread thread, ConsoleCallType type, uint64_t timestamp,
                                     const std::vector<RemoteObject> &arguments);
    void CallRuntimeExecutionContextCreated(PtThread thread);
    void CallRuntimeExecutionContextsCleared();
    void CallTargetAttachedToTarget(PtThread thread);
    void CallTargetDetachedFromTarget(PtThread thread);

    void OnCallDebuggerContinueToLocation(std::function<void(PtThread, std::string_view, size_t)> &&handler);
    void OnCallDebuggerGetPossibleBreakpoints(
        std::function<std::set<size_t>(std::string_view, size_t, size_t, bool)> &&handler);
    void OnCallDebuggerGetScriptSource(std::function<std::string(std::string_view)> &&handler);
    void OnCallDebuggerPause(std::function<void(PtThread)> &&handler);
    void OnCallDebuggerRemoveBreakpoint(std::function<void(PtThread, BreakpointId)> &&handler);
    void OnCallDebuggerRestartFrame(std::function<void(PtThread, FrameId)> &&handler);
    void OnCallDebuggerResume(std::function<void(PtThread)> &&handler);
    void OnCallDebuggerSetBreakpoint(std::function<SetBreakpointHandler> &&handler);
    void OnCallDebuggerSetBreakpointByUrl(std::function<SetBreakpointHandler> &&handler);
    void OnCallDebuggerSetBreakpointsActive(std::function<void(PtThread, bool)> &&handler);
    void OnCallDebuggerSetPauseOnExceptions(std::function<void(PtThread, PauseOnExceptionsState)> &&handler);
    void OnCallDebuggerStepInto(std::function<void(PtThread)> &&handler);
    void OnCallDebuggerStepOut(std::function<void(PtThread)> &&handler);
    void OnCallDebuggerStepOver(std::function<void(PtThread)> &&handler);

    void OnCallRuntimeEnable(std::function<void(PtThread)> &&handler);
    void OnCallRuntimeGetProperties(
        std::function<std::vector<PropertyDescriptor>(PtThread, RemoteObjectId, bool)> &&handler);
    void OnCallRuntimeRunIfWaitingForDebugger(std::function<void(PtThread)> &&handler);
    void OnCallRuntimeEvaluate(std::function<EvaluationResult(PtThread, const std::string &)> &&handler);

private:
    struct CallFrameInfo {
        FrameId frameId;
        std::string_view sourceFile;
        std::string_view methodName;
        size_t lineNumber;
    };

private:
    void SendTargetAttachedToTarget(const std::string &sessionId);
    void EnumerateCallFrames(JsonArrayBuilder &callFrames, PtThread thread,
                             const std::function<void(const FrameInfoHandler &)> &enumerateFrames);
    void AddCallFrameInfo(JsonArrayBuilder &callFrames, const CallFrameInfo &callFrameInfo,
                          const std::vector<Scope> &scopeChain, PtThread thread,
                          const std::optional<RemoteObject> &objThis);
    void AddBreakpointByUrlLocations(JsonArrayBuilder &locations, const std::set<std::string_view> &sourceFiles,
                                     size_t lineNumber, PtThread thread);
    static void AddHitBreakpoints(JsonArrayBuilder &hitBreakpointsBuilder,
                                  const std::vector<BreakpointId> &hitBreakpoints);
    static std::string GetExecutionContextUniqueId(const PtThread &thread);

    Server &server_;
    SessionManager sessionManager_;
    SourceManager sourceManager_;
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_INSPECTOR_SERVER_H
