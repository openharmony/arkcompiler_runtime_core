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

#include "inspector_server.h"

#include <functional>
#include <regex>
#include <string>
#include <utility>

#include "include/console_call_type.h"
#include "include/tooling/pt_thread.h"
#include "macros.h"
#include "utils/json_builder.h"
#include "utils/json_parser.h"
#include "utils/logger.h"

#include "connection/server.h"
#include "types/location.h"
#include "types/numeric_id.h"

namespace ark::tooling::inspector {
InspectorServer::InspectorServer(Server &server) : server_(server)
{
    server_.OnCall("Debugger.enable", [](auto, auto &result, auto &) { result.AddProperty("debuggerId", "debugger"); });
}

void InspectorServer::Kill()
{
    server_.Kill();
}

void InspectorServer::Run()
{
    server_.Run();
}

void InspectorServer::OnValidate(std::function<void()> &&handler)
{
    server_.OnValidate([handler = std::move(handler)]() {
        // Pause debugger events processing
        handler();

        // At this point, the whole messaging is stopped due to:
        // - Debugger events are not processed by the inspector after the call above;
        // - Client messages are not processed as we are executing on the server thread.
    });
}

void InspectorServer::OnOpen(std::function<void()> &&handler)
{
    server_.OnOpen([this, handler = std::move(handler)]() {
        // A new connection is open, reinitialize the state
        sessionManager_.EnumerateSessions([this](auto &id, auto thread) {
            sourceManager_.RemoveThread(thread);
            if (!id.empty()) {
                SendTargetAttachedToTarget(id);
            }
        });

        // Resume debugger events processing
        handler();
    });
}

void InspectorServer::OnFail(std::function<void()> &&handler)
{
    server_.OnFail([handler = std::move(handler)]() {
        // Resume debugger events processing
        handler();
    });
}

void InspectorServer::CallDebuggerPaused(PtThread thread, const std::vector<BreakpointId> &hitBreakpoints,
                                         const std::optional<RemoteObject> &exception,
                                         const std::function<void(const FrameInfoHandler &)> &enumerateFrames)
{
    auto sessionId = sessionManager_.GetSessionIdByThread(thread);

    server_.Call(sessionId, "Debugger.paused", [&](auto &params) {
        params.AddProperty("callFrames", [this, thread, &enumerateFrames](JsonArrayBuilder &callFrames) {
            EnumerateCallFrames(callFrames, thread, enumerateFrames);
        });

        params.AddProperty("hitBreakpoints", [&hitBreakpoints](JsonArrayBuilder &hitBreakpointsBuilder) {
            AddHitBreakpoints(hitBreakpointsBuilder, hitBreakpoints);
        });

        if (exception) {
            params.AddProperty("data", *exception);
        }

        params.AddProperty("reason", exception.has_value() ? "exception" : "other");
    });
}

void InspectorServer::CallDebuggerResumed(PtThread thread)
{
    server_.Call(sessionManager_.GetSessionIdByThread(thread), "Debugger.resumed");
}

void InspectorServer::CallDebuggerScriptParsed(PtThread thread, ScriptId scriptId, std::string_view sourceFile)
{
    auto sessionId = sessionManager_.GetSessionIdByThread(thread);
    server_.Call(sessionId, "Debugger.scriptParsed", [&sourceFile, &thread, &scriptId](auto &params) {
        params.AddProperty("executionContextId", thread.GetId());
        params.AddProperty("scriptId", std::to_string(scriptId));
        params.AddProperty("url", sourceFile.data());
        params.AddProperty("startLine", 0);
        params.AddProperty("startColumn", 0);
        params.AddProperty("endLine", std::numeric_limits<int>::max());
        params.AddProperty("endColumn", std::numeric_limits<int>::max());
        params.AddProperty("hash", "");
    });
}

void InspectorServer::CallRuntimeConsoleApiCalled(PtThread thread, ConsoleCallType type, uint64_t timestamp,
                                                  const std::vector<RemoteObject> &arguments)
{
    auto sessionId = sessionManager_.GetSessionIdByThread(thread);

    server_.Call(sessionId, "Runtime.consoleAPICalled", [&](auto &params) {
        params.AddProperty("executionContextId", thread.GetId());
        params.AddProperty("timestamp", timestamp);

        switch (type) {
            case ConsoleCallType::LOG:
                params.AddProperty("type", "log");
                break;
            case ConsoleCallType::DEBUG:
                params.AddProperty("type", "debug");
                break;
            case ConsoleCallType::INFO:
                params.AddProperty("type", "info");
                break;
            case ConsoleCallType::ERROR:
                params.AddProperty("type", "error");
                break;
            case ConsoleCallType::WARNING:
                params.AddProperty("type", "warning");
                break;
            default:
                UNREACHABLE();
        }

        params.AddProperty("args", [&](JsonArrayBuilder &argsBuilder) {
            for (const auto &argument : arguments) {
                argsBuilder.Add(argument);
            }
        });
    });
}

void InspectorServer::CallRuntimeExecutionContextCreated(PtThread thread)
{
    auto sessionId = sessionManager_.GetSessionIdByThread(thread);

    std::string name;
    if (thread != PtThread::NONE) {
        name = "Thread #" + std::to_string(thread.GetId());
    }

    server_.Call(sessionId, "Runtime.executionContextCreated", [&](auto &params) {
        params.AddProperty("context", [&](JsonObjectBuilder &context) {
            context.AddProperty("id", thread.GetId());
            context.AddProperty("origin", "");
            context.AddProperty("name", name);
            context.AddProperty("uniqueId", GetExecutionContextUniqueId(thread));
        });
    });
}

void InspectorServer::CallRuntimeExecutionContextsCleared()
{
    server_.Call("Runtime.executionContextsCleared");
}

void InspectorServer::CallTargetAttachedToTarget(PtThread thread)
{
    auto &sessionId = sessionManager_.AddSession(thread);
    if (!sessionId.empty()) {
        SendTargetAttachedToTarget(sessionId);
    }
}

void InspectorServer::CallTargetDetachedFromTarget(PtThread thread)
{
    auto sessionId = sessionManager_.GetSessionIdByThread(thread);

    // Pause the server thread to ensure that there will be no dangling PtThreads
    server_.Pause();

    sessionManager_.RemoveSession(sessionId);
    sourceManager_.RemoveThread(thread);

    // Now no one will retrieve the detached thread from the sessions manager
    server_.Continue();

    if (!sessionId.empty()) {
        server_.Call("Target.detachedFromTarget",
                     [&sessionId](auto &params) { params.AddProperty("session_id", sessionId); });
    }
}

void InspectorServer::OnCallDebuggerContinueToLocation(
    std::function<void(PtThread, std::string_view, size_t)> &&handler)
{
    server_.OnCall(
        "Debugger.continueToLocation", [this, handler = std::move(handler)](auto &sessionId, auto &, auto &params) {
            auto location = Location::FromJsonProperty(params, "location");
            if (!location) {
                LOG(INFO, DEBUGGER) << location.Error();
                return;
            }

            auto thread = sessionManager_.GetThreadBySessionId(sessionId);

            handler(thread, sourceManager_.GetSourceFileName(location->GetScriptId()), location->GetLineNumber());
        });
}

void InspectorServer::OnCallDebuggerGetPossibleBreakpoints(
    std::function<std::set<size_t>(std::string_view, size_t, size_t, bool)> &&handler)
{
    // clang-format off
    server_.OnCall("Debugger.getPossibleBreakpoints",
        [this, handler = std::move(handler)](auto &, auto &result, const JsonObject &params) {
            auto start = Location::FromJsonProperty(params, "start");
            if (!start) {
                LOG(INFO, DEBUGGER) << start.Error();
                return;
            }

            auto scriptId = start->GetScriptId();

            size_t endLine = ~0U;
            if (auto end = Location::FromJsonProperty(params, "end")) {
                if (end->GetScriptId() != scriptId) {
                    LOG(INFO, DEBUGGER) << "Script ids don't match";
                    return;
                }

                endLine = end->GetLineNumber();
            }

            bool restrictToFunction = false;
            if (auto prop = params.GetValue<JsonObject::BoolT>("restrictToFunction")) {
                restrictToFunction = *prop;
            }

            auto lineNumbers = handler(sourceManager_.GetSourceFileName(scriptId), start->GetLineNumber(),
                                       endLine, restrictToFunction);

            result.AddProperty("locations", [scriptId, &lineNumbers](JsonArrayBuilder &array) {
                for (auto lineNumber : lineNumbers) {
                    array.Add(Location(scriptId, lineNumber));
                }
            });
        });
    // clang-format on
}

void InspectorServer::OnCallDebuggerGetScriptSource(std::function<std::string(std::string_view)> &&handler)
{
    server_.OnCall("Debugger.getScriptSource",
                   [this, handler = std::move(handler)](auto &, auto &result, auto &params) {
                       if (auto scriptId = ParseNumericId<ScriptId>(params, "scriptId")) {
                           auto sourceFile = sourceManager_.GetSourceFileName(*scriptId);
                           result.AddProperty("scriptSource", handler(sourceFile));
                       } else {
                           LOG(INFO, DEBUGGER) << scriptId.Error();
                       }
                   });
}

void InspectorServer::OnCallDebuggerPause(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.pause", [this, handler = std::move(handler)](auto &sessionId, auto &, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
    });
}

void InspectorServer::OnCallDebuggerRemoveBreakpoint(std::function<void(PtThread, BreakpointId)> &&handler)
{
    server_.OnCall("Debugger.removeBreakpoint",
                   [this, handler = std::move(handler)](auto &sessionId, auto &, auto &params) {
                       if (auto breakpointId = ParseNumericId<BreakpointId>(params, "breakpointId")) {
                           handler(sessionManager_.GetThreadBySessionId(sessionId), *breakpointId);
                       } else {
                           LOG(INFO, DEBUGGER) << breakpointId.Error();
                       }
                   });
}

void InspectorServer::OnCallDebuggerRestartFrame(std::function<void(PtThread, FrameId)> &&handler)
{
    // clang-format off
    server_.OnCall("Debugger.restartFrame",
        [this, handler = std::move(handler)](auto &sessionId, auto &result, auto &params) {
            auto thread = sessionManager_.GetThreadBySessionId(sessionId);

            auto frameId = ParseNumericId<FrameId>(params, "callFrameId");
            if (!frameId) {
                LOG(INFO, DEBUGGER) << frameId.Error();
                return;
            }

            handler(thread, *frameId);

            result.AddProperty("callFrames", [](JsonArrayBuilder &) {});
        });
    // clang-format on
}

void InspectorServer::OnCallDebuggerResume(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.resume", [this, handler = std::move(handler)](auto &sessionId, auto &, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
    });
}

void InspectorServer::OnCallDebuggerSetBreakpoint(std::function<SetBreakpointHandler> &&handler)
{
    // clang-format off
    server_.OnCall("Debugger.setBreakpoint",
        [this, handler = std::move(handler)](auto &sessionId, auto &result, auto &params) {
            auto location = Location::FromJsonProperty(params, "location");
            if (!location) {
                LOG(INFO, DEBUGGER) << location.Error();
                return;
            }
            auto condition = params.template GetValue<JsonObject::StringT>("condition");

            auto thread = sessionManager_.GetThreadBySessionId(sessionId);

            auto sourceFile = sourceManager_.GetSourceFileName(location->GetScriptId());
            std::set<std::string_view> sourceFiles;

            auto id = handler(
                thread, [sourceFile](auto fileName) { return fileName == sourceFile; },
                location->GetLineNumber(), sourceFiles, condition);
            if (!id) {
                LOG(INFO, DEBUGGER) << "Failed to set breakpoint";
                return;
            }

            result.AddProperty("breakpointId", std::to_string(*id));
            result.AddProperty("actualLocation", *location);
        });
    // clang-format on
}

void InspectorServer::OnCallDebuggerSetBreakpointByUrl(std::function<SetBreakpointHandler> &&handler)
{
    server_.OnCall("Debugger.setBreakpointByUrl", [this, handler = std::move(handler)](auto &sessionId, auto &result,
                                                                                       const JsonObject &params) {
        size_t lineNumber;
        if (auto prop = params.GetValue<JsonObject::NumT>("lineNumber")) {
            lineNumber = *prop + 1;
        } else {
            LOG(INFO, DEBUGGER) << "No 'lineNumber' property";
            return;
        }

        std::function<bool(std::string_view)> sourceFileFilter;
        if (auto url = params.GetValue<JsonObject::StringT>("url")) {
            static constexpr std::string_view FILE_PREFIX = "file://";
            sourceFileFilter = [sourceFile = url->find(FILE_PREFIX) == 0 ? url->substr(FILE_PREFIX.size()) : *url](
                                   auto fileName) { return fileName == sourceFile; };
        } else if (auto urlRegex = params.GetValue<JsonObject::StringT>("urlRegex")) {
            sourceFileFilter = [regex = std::regex(*urlRegex)](auto fileName) {
                return std::regex_match(fileName.data(), regex);
            };
        } else {
            LOG(INFO, DEBUGGER) << "No 'url' or 'urlRegex' properties";
            return;
        }

        auto condition = params.template GetValue<JsonObject::StringT>("condition");

        std::set<std::string_view> sourceFiles;
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);

        auto id = handler(thread, sourceFileFilter, lineNumber, sourceFiles, condition);
        if (!id) {
            LOG(INFO, DEBUGGER) << "Failed to set breakpoint";
            return;
        }

        result.AddProperty("breakpointId", std::to_string(*id));
        result.AddProperty("locations", [this, lineNumber, &sourceFiles, thread](JsonArrayBuilder &locations) {
            AddBreakpointByUrlLocations(locations, sourceFiles, lineNumber, thread);
        });
    });
}

void InspectorServer::OnCallDebuggerSetBreakpointsActive(std::function<void(PtThread, bool)> &&handler)
{
    // clang-format off
    server_.OnCall("Debugger.setBreakpointsActive",
        [this, handler = std::move(handler)](auto &sessionId, auto &, const JsonObject &params) {
            bool active;
            if (auto prop = params.GetValue<JsonObject::BoolT>("active")) {
                active = *prop;
            } else {
                LOG(INFO, DEBUGGER) << "No 'active' property";
                return;
            }

            auto thread = sessionManager_.GetThreadBySessionId(sessionId);
            handler(thread, active);
        });
    // clang-format on
}

void InspectorServer::OnCallDebuggerSetPauseOnExceptions(
    std::function<void(PtThread, PauseOnExceptionsState)> &&handler)
{
    // clang-format off
    server_.OnCall("Debugger.setPauseOnExceptions",
        [this, handler = std::move(handler)](auto &sessionId, auto &, const JsonObject &params) {
            auto thread = sessionManager_.GetThreadBySessionId(sessionId);

            PauseOnExceptionsState state;
            auto stateStr = params.GetValue<JsonObject::StringT>("state");
            if (stateStr == nullptr) {
                LOG(INFO, DEBUGGER) << "No 'state' property";
                return;
            }

            if (*stateStr == "none") {
                state = PauseOnExceptionsState::NONE;
            } else if (*stateStr == "caught") {
                state = PauseOnExceptionsState::CAUGHT;
            } else if (*stateStr == "uncaught") {
                state = PauseOnExceptionsState::UNCAUGHT;
            } else if (*stateStr == "all") {
                state = PauseOnExceptionsState::ALL;
            } else {
                LOG(INFO, DEBUGGER) << "Invalid 'state' value: " << *stateStr;
                return;
            }

            handler(thread, state);
        });
    // clang-format on
}

void InspectorServer::OnCallDebuggerStepInto(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.stepInto", [this, handler = std::move(handler)](auto &sessionId, auto &, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
    });
}

void InspectorServer::OnCallDebuggerStepOut(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.stepOut", [this, handler = std::move(handler)](auto &sessionId, auto &, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
    });
}

void InspectorServer::OnCallDebuggerStepOver(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.stepOver", [this, handler = std::move(handler)](auto &sessionId, auto &, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
    });
}

void InspectorServer::OnCallRuntimeEnable(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Runtime.enable", [this, handler = std::move(handler)](auto &sessionId, auto &, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
    });
}

void InspectorServer::OnCallRuntimeGetProperties(
    std::function<std::vector<PropertyDescriptor>(PtThread, RemoteObjectId, bool)> &&handler)
{
    // clang-format off
    server_.OnCall("Runtime.getProperties",
        [this, handler = std::move(handler)](auto &sessionId, auto &result, const JsonObject &params) {
            auto thread = sessionManager_.GetThreadBySessionId(sessionId);

            auto objectId = ParseNumericId<RemoteObjectId>(params, "objectId");
            if (!objectId) {
                LOG(INFO, DEBUGGER) << objectId.Error();
                return;
            }

            auto generatePreview = false;
            if (auto prop = params.GetValue<JsonObject::BoolT>("generatePreview")) {
                generatePreview = *prop;
            }

            result.AddProperty("result", [&](JsonArrayBuilder &array) {
                for (auto &descriptor : handler(thread, *objectId, generatePreview)) {
                    array.Add(descriptor);
                }
            });
        });
    // clang-format on
}

void InspectorServer::OnCallRuntimeRunIfWaitingForDebugger(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Runtime.runIfWaitingForDebugger",
                   [this, handler = std::move(handler)](auto &sessionId, auto &, auto &) {
                       auto thread = sessionManager_.GetThreadBySessionId(sessionId);
                       handler(thread);
                   });
}

void InspectorServer::OnCallRuntimeEvaluate(std::function<EvaluationResult(PtThread, const std::string &)> &&handler)
{
    // clang-format off
    server_.OnCall("Runtime.evaluate",
        [this, handler = std::move(handler)](auto &sessionId, auto &result, const JsonObject &params) {
            auto thread = sessionManager_.GetThreadBySessionId(sessionId);

            auto expressionStr = params.GetValue<JsonObject::StringT>("expression");
            if (expressionStr == nullptr || expressionStr->empty()) {
                LOG(INFO, DEBUGGER) << "'expression' property is absent or empty";
                return;
            }

            auto optResult = handler(thread, *expressionStr);
            if (!optResult) {
                // NOTE(dslynko): might return error instead of `undefined`.
                LOG(DEBUG, DEBUGGER) << "Evaluation failed";
                result.AddProperty("result", RemoteObject::Undefined());
                return;
            }

            auto [evalResult, exceptionDetails] = *optResult;
            result.AddProperty("result", evalResult);
            if (exceptionDetails) {
                result.AddProperty("exceptionDetails", *exceptionDetails);
            }
        });
    // clang-format on
}

void InspectorServer::SendTargetAttachedToTarget(const std::string &sessionId)
{
    server_.Call("Target.attachedToTarget", [&sessionId](auto &params) {
        params.AddProperty("sessionId", sessionId);
        params.AddProperty("targetInfo", [&sessionId](JsonObjectBuilder &targetInfo) {
            targetInfo.AddProperty("targetId", sessionId);
            targetInfo.AddProperty("type", "worker");
            targetInfo.AddProperty("title", sessionId);
            targetInfo.AddProperty("url", "");
            targetInfo.AddProperty("attached", true);
            targetInfo.AddProperty("canAccessOpener", false);
        });
        params.AddProperty("waitingForDebugger", true);
    });
}

void InspectorServer::EnumerateCallFrames(JsonArrayBuilder &callFrames, PtThread thread,
                                          const std::function<void(const FrameInfoHandler &)> &enumerateFrames)
{
    enumerateFrames([this, thread, &callFrames](auto frameId, auto methodName, auto sourceFile, auto lineNumber,
                                                auto &scopeChain, auto &objThis) {
        CallFrameInfo callFrameInfo {frameId, sourceFile, methodName, lineNumber};
        AddCallFrameInfo(callFrames, callFrameInfo, scopeChain, thread, objThis);
    });
}

void InspectorServer::AddCallFrameInfo(JsonArrayBuilder &callFrames, const CallFrameInfo &callFrameInfo,
                                       const std::vector<Scope> &scopeChain, PtThread thread,
                                       const std::optional<RemoteObject> &objThis)
{
    callFrames.Add([&](JsonObjectBuilder &callFrame) {
        auto [scriptId, isNew] = sourceManager_.GetScriptId(thread, callFrameInfo.sourceFile);

        if (isNew) {
            CallDebuggerScriptParsed(thread, scriptId, callFrameInfo.sourceFile);
        }

        callFrame.AddProperty("callFrameId", std::to_string(callFrameInfo.frameId));
        callFrame.AddProperty("functionName", callFrameInfo.methodName.data());
        callFrame.AddProperty("location", Location(scriptId, callFrameInfo.lineNumber));
        callFrame.AddProperty("url", callFrameInfo.sourceFile.data());

        callFrame.AddProperty("scopeChain", [&](JsonArrayBuilder &scopeChainBuilder) {
            for (auto &scope : scopeChain) {
                scopeChainBuilder.Add(scope);
            }
        });

        callFrame.AddProperty("this", objThis.value_or(RemoteObject::Undefined()));
        callFrame.AddProperty("canBeRestarted", true);
    });
}

void InspectorServer::AddBreakpointByUrlLocations(JsonArrayBuilder &locations,
                                                  const std::set<std::string_view> &sourceFiles, size_t lineNumber,
                                                  PtThread thread)
{
    for (auto sourceFile : sourceFiles) {
        auto [scriptId, isNew] = sourceManager_.GetScriptId(thread, sourceFile);

        if (isNew) {
            CallDebuggerScriptParsed(thread, scriptId, sourceFile);
        }

        locations.Add(Location {scriptId, lineNumber});
    }
}

/* static */
void InspectorServer::AddHitBreakpoints(JsonArrayBuilder &hitBreakpointsBuilder,
                                        const std::vector<BreakpointId> &hitBreakpoints)
{
    for (auto id : hitBreakpoints) {
        hitBreakpointsBuilder.Add(std::to_string(id));
    }
}

/* static */
std::string InspectorServer::GetExecutionContextUniqueId(const PtThread &thread)
{
    static int pid = os::thread::GetPid();
    std::stringstream ss;
    ss << pid << ':' << thread.GetId();
    return ss.str();
}
}  // namespace ark::tooling::inspector
