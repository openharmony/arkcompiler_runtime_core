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

#include "inspector_server.h"

#include "server.h"
#include "types/location.h"
#include "types/numeric_id.h"

#include "console_call_type.h"
#include "macros.h"
#include "tooling/pt_thread.h"
#include "utils/json_builder.h"
#include "utils/json_parser.h"
#include "utils/logger.h"

#include <functional>
#include <regex>
#include <string>
#include <utility>

namespace panda::tooling::inspector {
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
        session_manager_.EnumerateSessions([this](auto &id, auto thread) {
            source_manager_.RemoveThread(thread);
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

void InspectorServer::CallDebuggerPaused(
    PtThread thread, const std::vector<BreakpointId> &hit_breakpoints, const std::optional<RemoteObject> &exception,
    const std::function<void(const std::function<void(FrameId, std::string_view, std::string_view, size_t,
                                                      const std::vector<Scope> &)> &)> &enumerate_frames)
{
    auto session_id = session_manager_.GetSessionIdByThread(thread);

    server_.Call(session_id, "Debugger.paused", [&](auto &params) {
        params.AddProperty("callFrames", [this, thread, &enumerate_frames](JsonArrayBuilder &call_frames) {
            enumerate_frames([this, thread, &call_frames](auto frame_id, auto method_name, auto source_file,
                                                          auto line_number, auto &scope_chain) {
                call_frames.Add([&](JsonObjectBuilder &call_frame) {
                    auto [script_id, is_new] = source_manager_.GetScriptId(thread, source_file);

                    if (is_new) {
                        CallDebuggerScriptParsed(thread, script_id, source_file);
                    }

                    call_frame.AddProperty("callFrameId", std::to_string(frame_id));
                    call_frame.AddProperty("functionName", method_name.data());
                    call_frame.AddProperty("location", Location(script_id, line_number).ToJson());
                    call_frame.AddProperty("url", source_file.data());

                    call_frame.AddProperty("scopeChain", [&](JsonArrayBuilder &scope_chain_builder) {
                        for (auto &scope : scope_chain) {
                            scope_chain_builder.Add(scope.ToJson());
                        }
                    });

                    call_frame.AddProperty("canBeRestarted", true);
                });
            });
        });

        params.AddProperty("hitBreakpoints", [&hit_breakpoints](JsonArrayBuilder &hit_breakpoints_builder) {
            for (auto id : hit_breakpoints) {
                hit_breakpoints_builder.Add(std::to_string(id));
            }
        });

        if (exception) {
            params.AddProperty("data", exception->ToJson());
        }

        params.AddProperty("reason", exception.has_value() ? "exception" : "other");
    });
}

void InspectorServer::CallDebuggerResumed(PtThread thread)
{
    server_.Call(session_manager_.GetSessionIdByThread(thread), "Debugger.resumed");
}

void InspectorServer::CallDebuggerScriptParsed(PtThread thread, ScriptId script_id, std::string_view source_file)
{
    auto session_id = session_manager_.GetSessionIdByThread(thread);
    server_.Call(session_id, "Debugger.scriptParsed", [&](auto &params) {
        params.AddProperty("executionContextId", thread.GetId());
        params.AddProperty("scriptId", std::to_string(script_id));
        params.AddProperty("url", source_file.data());
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
    auto session_id = session_manager_.GetSessionIdByThread(thread);

    server_.Call(session_id, "Runtime.consoleAPICalled", [&](auto &params) {
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

        params.AddProperty("args", [&](JsonArrayBuilder &args_builder) {
            for (const auto &argument : arguments) {
                args_builder.Add(argument.ToJson());
            }
        });
    });
}

void InspectorServer::CallRuntimeExecutionContextCreated(PtThread thread)
{
    auto session_id = session_manager_.GetSessionIdByThread(thread);

    std::string name;
    if (thread != PtThread::NONE) {
        name = "Thread #" + std::to_string(thread.GetId());
    }

    server_.Call(session_id, "Runtime.executionContextCreated", [&](auto &params) {
        params.AddProperty("context", [&](JsonObjectBuilder &context) {
            context.AddProperty("id", thread.GetId());
            context.AddProperty("origin", "");
            context.AddProperty("name", name);
        });
    });
}

void InspectorServer::CallTargetAttachedToTarget(PtThread thread)
{
    auto &session_id = session_manager_.AddSession(thread);
    if (!session_id.empty()) {
        SendTargetAttachedToTarget(session_id);
    }
}

void InspectorServer::CallTargetDetachedFromTarget(PtThread thread)
{
    auto session_id = session_manager_.GetSessionIdByThread(thread);

    // Pause the server thread to ensure that there will be no dangling PtThreads
    server_.Pause();

    session_manager_.RemoveSession(session_id);
    source_manager_.RemoveThread(thread);

    // Now no one will retrieve the detached thread from the sessions manager
    server_.Continue();

    if (!session_id.empty()) {
        server_.Call("Target.detachedFromTarget",
                     [&session_id](auto &params) { params.AddProperty("session_id", session_id); });
    }
}

void InspectorServer::OnCallDebuggerContinueToLocation(
    std::function<void(PtThread, std::string_view, size_t)> &&handler)
{
    server_.OnCall(
        "Debugger.continueToLocation", [this, handler = std::move(handler)](auto &session_id, auto &, auto &params) {
            auto location = Location::FromJsonProperty(params, "location");
            if (!location) {
                LOG(INFO, DEBUGGER) << location.Error();
                return;
            }

            auto thread = session_manager_.GetThreadBySessionId(session_id);

            handler(thread, source_manager_.GetSourceFileName(location->GetScriptId()), location->GetLineNumber());
        });
}

void InspectorServer::OnCallDebuggerGetPossibleBreakpoints(
    std::function<std::set<size_t>(std::string_view, size_t, size_t, bool)> &&handler)
{
    server_.OnCall("Debugger.getPossibleBreakpoints",
                   [this, handler = std::move(handler)](auto &, auto &result, const JsonObject &params) {
                       auto start = Location::FromJsonProperty(params, "start");
                       if (!start) {
                           LOG(INFO, DEBUGGER) << start.Error();
                           return;
                       }

                       auto script_id = start->GetScriptId();

                       size_t end_line = ~0U;
                       if (auto end = Location::FromJsonProperty(params, "end")) {
                           if (end->GetScriptId() != script_id) {
                               LOG(INFO, DEBUGGER) << "Script ids don't match";
                               return;
                           }

                           end_line = end->GetLineNumber();
                       }

                       bool restrict_to_function = false;
                       if (auto prop = params.GetValue<JsonObject::BoolT>("restrictToFunction")) {
                           restrict_to_function = *prop;
                       }

                       auto line_numbers = handler(source_manager_.GetSourceFileName(script_id), start->GetLineNumber(),
                                                   end_line, restrict_to_function);

                       result.AddProperty("locations", [script_id, &line_numbers](JsonArrayBuilder &array) {
                           for (auto line_number : line_numbers) {
                               array.Add(Location(script_id, line_number).ToJson());
                           }
                       });
                   });
}

void InspectorServer::OnCallDebuggerGetScriptSource(std::function<std::string(std::string_view)> &&handler)
{
    server_.OnCall("Debugger.getScriptSource",
                   [this, handler = std::move(handler)](auto &, auto &result, auto &params) {
                       if (auto script_id = ParseNumericId<ScriptId>(params, "scriptId")) {
                           auto source_file = source_manager_.GetSourceFileName(*script_id);
                           result.AddProperty("scriptSource", handler(source_file));
                       } else {
                           LOG(INFO, DEBUGGER) << script_id.Error();
                       }
                   });
}

void InspectorServer::OnCallDebuggerPause(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.pause", [this, handler = std::move(handler)](auto &session_id, auto &, auto &) {
        auto thread = session_manager_.GetThreadBySessionId(session_id);
        handler(thread);
    });
}

void InspectorServer::OnCallDebuggerRemoveBreakpoint(std::function<void(PtThread, BreakpointId)> &&handler)
{
    server_.OnCall("Debugger.removeBreakpoint",
                   [this, handler = std::move(handler)](auto &session_id, auto &, auto &params) {
                       if (auto breakpoint_id = ParseNumericId<BreakpointId>(params, "breakpointId")) {
                           handler(session_manager_.GetThreadBySessionId(session_id), *breakpoint_id);
                       } else {
                           LOG(INFO, DEBUGGER) << breakpoint_id.Error();
                       }
                   });
}

void InspectorServer::OnCallDebuggerRestartFrame(std::function<void(PtThread, FrameId)> &&handler)
{
    server_.OnCall("Debugger.restartFrame",
                   [this, handler = std::move(handler)](auto &session_id, auto &result, auto &params) {
                       auto thread = session_manager_.GetThreadBySessionId(session_id);

                       auto frame_id = ParseNumericId<FrameId>(params, "callFrameId");
                       if (!frame_id) {
                           LOG(INFO, DEBUGGER) << frame_id.Error();
                           return;
                       }

                       handler(thread, *frame_id);

                       result.AddProperty("callFrames", [](JsonArrayBuilder &) {});
                   });
}

void InspectorServer::OnCallDebuggerResume(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.resume", [this, handler = std::move(handler)](auto &session_id, auto &, auto &) {
        auto thread = session_manager_.GetThreadBySessionId(session_id);
        handler(thread);
    });
}

void InspectorServer::OnCallDebuggerSetBreakpoint(
    std::function<std::optional<BreakpointId>(PtThread, const std::function<bool(std::string_view)> &, size_t,
                                              std::set<std::string_view> &)> &&handler)
{
    server_.OnCall("Debugger.setBreakpoint",
                   [this, handler = std::move(handler)](auto &session_id, auto &result, auto &params) {
                       auto location = Location::FromJsonProperty(params, "location");
                       if (!location) {
                           LOG(INFO, DEBUGGER) << location.Error();
                           return;
                       }

                       auto thread = session_manager_.GetThreadBySessionId(session_id);

                       auto source_file = source_manager_.GetSourceFileName(location->GetScriptId());
                       std::set<std::string_view> source_files;

                       auto id = handler(
                           thread, [source_file](auto file_name) { return file_name == source_file; },
                           location->GetLineNumber(), source_files);
                       if (!id) {
                           LOG(INFO, DEBUGGER) << "Failed to set breakpoint";
                           return;
                       }

                       result.AddProperty("breakpointId", std::to_string(*id));
                       result.AddProperty("actualLocation", location->ToJson());
                   });
}

void InspectorServer::OnCallDebuggerSetBreakpointByUrl(
    std::function<std::optional<BreakpointId>(PtThread, const std::function<bool(std::string_view)> &, size_t,
                                              std::set<std::string_view> &)> &&handler)
{
    server_.OnCall("Debugger.setBreakpointByUrl", [this, handler = std::move(handler)](auto &session_id, auto &result,
                                                                                       const JsonObject &params) {
        size_t line_number;
        if (auto prop = params.GetValue<JsonObject::NumT>("lineNumber")) {
            line_number = *prop + 1;
        } else {
            LOG(INFO, DEBUGGER) << "No 'lineNumber' property";
            return;
        }

        std::function<bool(std::string_view)> source_file_filter;
        if (auto url = params.GetValue<JsonObject::StringT>("url")) {
            source_file_filter = [source_file = url->find("file://") == 0 ? url->substr(std::strlen("file://")) : *url](
                                     auto file_name) { return file_name == source_file; };
        } else if (auto url_regex = params.GetValue<JsonObject::StringT>("urlRegex")) {
            source_file_filter = [regex = std::regex(*url_regex)](auto file_name) {
                return std::regex_match(file_name.data(), regex);
            };
        } else {
            LOG(INFO, DEBUGGER) << "No 'url' or 'urlRegex' properties";
            return;
        }

        std::set<std::string_view> source_files;
        auto thread = session_manager_.GetThreadBySessionId(session_id);

        auto id = handler(thread, source_file_filter, line_number, source_files);
        if (!id) {
            LOG(INFO, DEBUGGER) << "Failed to set breakpoint";
            return;
        }

        result.AddProperty("breakpointId", std::to_string(*id));
        result.AddProperty("locations", [this, line_number, &source_files, thread](JsonArrayBuilder &locations) {
            for (auto source_file : source_files) {
                locations.Add([this, line_number, thread, source_file](JsonObjectBuilder &location) {
                    auto [script_id, is_new] = source_manager_.GetScriptId(thread, source_file);

                    if (is_new) {
                        CallDebuggerScriptParsed(thread, script_id, source_file);
                    }

                    Location(script_id, line_number).ToJson()(location);
                });
            }
        });
    });
}

void InspectorServer::OnCallDebuggerSetBreakpointsActive(std::function<void(PtThread, bool)> &&handler)
{
    server_.OnCall("Debugger.setBreakpointsActive",
                   [this, handler = std::move(handler)](auto &session_id, auto &, const JsonObject &params) {
                       bool active;
                       if (auto prop = params.GetValue<JsonObject::BoolT>("active")) {
                           active = *prop;
                       } else {
                           LOG(INFO, DEBUGGER) << "No 'active' property";
                           return;
                       }

                       auto thread = session_manager_.GetThreadBySessionId(session_id);
                       handler(thread, active);
                   });
}

void InspectorServer::OnCallDebuggerSetPauseOnExceptions(
    std::function<void(PtThread, PauseOnExceptionsState)> &&handler)
{
    server_.OnCall("Debugger.setPauseOnExceptions",
                   [this, handler = std::move(handler)](auto &session_id, auto &, const JsonObject &params) {
                       auto thread = session_manager_.GetThreadBySessionId(session_id);

                       PauseOnExceptionsState state;
                       auto state_str = params.GetValue<JsonObject::StringT>("state");
                       if (state_str == nullptr) {
                           LOG(INFO, DEBUGGER) << "No 'state' property";
                           return;
                       }

                       if (*state_str == "none") {
                           state = PauseOnExceptionsState::NONE;
                       } else if (*state_str == "caught") {
                           state = PauseOnExceptionsState::CAUGHT;
                       } else if (*state_str == "uncaught") {
                           state = PauseOnExceptionsState::UNCAUGHT;
                       } else if (*state_str == "all") {
                           state = PauseOnExceptionsState::ALL;
                       } else {
                           LOG(INFO, DEBUGGER) << "Invalid 'state' value: " << *state_str;
                           return;
                       }

                       handler(thread, state);
                   });
}

void InspectorServer::OnCallDebuggerStepInto(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.stepInto", [this, handler = std::move(handler)](auto &session_id, auto &, auto &) {
        auto thread = session_manager_.GetThreadBySessionId(session_id);
        handler(thread);
    });
}

void InspectorServer::OnCallDebuggerStepOut(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.stepOut", [this, handler = std::move(handler)](auto &session_id, auto &, auto &) {
        auto thread = session_manager_.GetThreadBySessionId(session_id);
        handler(thread);
    });
}

void InspectorServer::OnCallDebuggerStepOver(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.stepOver", [this, handler = std::move(handler)](auto &session_id, auto &, auto &) {
        auto thread = session_manager_.GetThreadBySessionId(session_id);
        handler(thread);
    });
}

void InspectorServer::OnCallRuntimeEnable(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Runtime.enable", [this, handler = std::move(handler)](auto &session_id, auto &, auto &) {
        auto thread = session_manager_.GetThreadBySessionId(session_id);
        handler(thread);
    });
}

void InspectorServer::OnCallRuntimeGetProperties(
    std::function<std::vector<PropertyDescriptor>(PtThread, RemoteObjectId, bool)> &&handler)
{
    server_.OnCall("Runtime.getProperties",
                   [this, handler = std::move(handler)](auto &session_id, auto &result, const JsonObject &params) {
                       auto thread = session_manager_.GetThreadBySessionId(session_id);

                       auto object_id = ParseNumericId<RemoteObjectId>(params, "objectId");
                       if (!object_id) {
                           LOG(INFO, DEBUGGER) << object_id.Error();
                           return;
                       }

                       auto generate_preview = false;
                       if (auto prop = params.GetValue<JsonObject::BoolT>("generatePreview")) {
                           generate_preview = *prop;
                       }

                       result.AddProperty("result", [&](JsonArrayBuilder &array) {
                           for (auto &descriptor : handler(thread, *object_id, generate_preview)) {
                               array.Add(descriptor.ToJson());
                           }
                       });
                   });
}

void InspectorServer::OnCallRuntimeRunIfWaitingForDebugger(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Runtime.runIfWaitingForDebugger",
                   [this, handler = std::move(handler)](auto &session_id, auto &, auto &) {
                       auto thread = session_manager_.GetThreadBySessionId(session_id);
                       handler(thread);
                   });
}

void InspectorServer::SendTargetAttachedToTarget(const std::string &session_id)
{
    server_.Call("Target.attachedToTarget", [&session_id](auto &params) {
        params.AddProperty("sessionId", session_id);
        params.AddProperty("targetInfo", [&session_id](JsonObjectBuilder &target_info) {
            target_info.AddProperty("targetId", session_id);
            target_info.AddProperty("type", "worker");
            target_info.AddProperty("title", session_id);
            target_info.AddProperty("url", "");
            target_info.AddProperty("attached", true);
            target_info.AddProperty("canAccessOpener", false);
        });
        params.AddProperty("waitingForDebugger", true);
    });
}
}  // namespace panda::tooling::inspector
