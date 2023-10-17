/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "common.h"
#include "types/location.h"
#include "inspector_server.h"
#include "server.h"
#include "utils/json_builder.h"
#include "json_object_matcher.h"
#include "runtime.h"
#include "assembly-emitter.h"
#include "assembly-parser.h"

// NOLINTBEGIN

namespace panda::tooling::inspector::test {

class TestServer : public Server {
public:
    void OnValidate([[maybe_unused]] std::function<void()> &&handler) override {};
    void OnOpen([[maybe_unused]] std::function<void()> &&handler) override {};
    void OnFail([[maybe_unused]] std::function<void()> &&handler) override {};

    void Call(const std::string &session, const char *method_call,
              std::function<void(JsonObjectBuilder &)> &&parameters) override
    {
        std::string tmp(method_call);
        CallMock(session, tmp, std::move(parameters));
    }

    void OnCall(const char *method_call,
                std::function<void(const std::string &session_id, JsonObjectBuilder &result, const JsonObject &params)>
                    &&handler) override
    {
        std::string tmp(method_call);
        OnCallMock(tmp, std::move(handler));
    }

    MOCK_METHOD(void, CallMock,
                (const std::string &session, const std::string &method_call,
                 std::function<void(JsonObjectBuilder &)> &&parameters));

    MOCK_METHOD(void, OnCallMock,
                (const std::string &method_call,
                 std::function<void(const std::string &session_id, JsonObjectBuilder &result, const JsonObject &params)>
                     &&handler));

    bool Poll() override
    {
        return true;
    };
    bool RunOne() override
    {
        return true;
    };
};

PtThread mthread = PtThread(PtThread::NONE);
const std::string session_id;
const std::string source_file = "source";
bool handler_called = false;

class ServerTest : public testing::Test {
public:
    void SetUp() override
    {
        RuntimeOptions options;
        options.SetShouldInitializeIntrinsics(false);
        options.SetShouldLoadBootPandaFiles(false);
        Runtime::Create(options);
        mthread = PtThread(ManagedThread::GetCurrent());
        handler_called = false;
    }
    void TearDown() override
    {
        Runtime::Destroy();
    }
    TestServer server;
    InspectorServer inspector_server {server};
};

TEST_F(ServerTest, CallDebuggerResumed)
{
    inspector_server.CallTargetAttachedToTarget(mthread);
    EXPECT_CALL(server, CallMock(session_id, "Debugger.resumed", testing::_)).Times(1);

    inspector_server.CallDebuggerResumed(mthread);
}

TEST_F(ServerTest, CallDebuggerScriptParsed)
{
    inspector_server.CallTargetAttachedToTarget(mthread);

    size_t script_id = 4;
    EXPECT_CALL(server, CallMock(session_id, "Debugger.scriptParsed", testing::_))
        .WillOnce([&](testing::Unused, testing::Unused, auto s) {
            ASSERT_THAT(ToObject(std::move(s)),
                        JsonProperties(JsonProperty<JsonObject::NumT> {"executionContextId", mthread.GetId()},
                                       JsonProperty<JsonObject::StringT> {"scriptId", std::to_string(script_id)},
                                       JsonProperty<JsonObject::NumT> {"startLine", 0},
                                       JsonProperty<JsonObject::NumT> {"startColumn", 0},
                                       JsonProperty<JsonObject::NumT> {"endLine", std::numeric_limits<int>::max()},
                                       JsonProperty<JsonObject::NumT> {"endColumn", std::numeric_limits<int>::max()},
                                       JsonProperty<JsonObject::StringT> {"hash", ""},
                                       JsonProperty<JsonObject::StringT> {"url", source_file.c_str()}));
        });
    inspector_server.CallDebuggerScriptParsed(mthread, ScriptId(script_id), source_file);
}

TEST_F(ServerTest, DebuggerEnable)
{
    TestServer server1;
    EXPECT_CALL(server1, OnCallMock("Debugger.enable", testing::_)).WillOnce([&](testing::Unused, auto handler) {
        JsonObjectBuilder res;
        JsonObject empty;
        handler(session_id, res, empty);
        ASSERT_THAT(JsonObject(std::move(res).Build()),
                    JsonProperties(JsonProperty<JsonObject::StringT> {"debuggerId", "debugger"}));
    });
    InspectorServer inspector_server1(server1);
}

auto simple_handler = []([[maybe_unused]] auto unused, auto handler) {
    JsonObjectBuilder res;
    JsonObject empty;
    handler(session_id, res, empty);
    ASSERT_THAT(JsonObject(std::move(res).Build()), JsonProperties());
    ASSERT_TRUE(handler_called);
};

TEST_F(ServerTest, OnCallDebuggerPause)
{
    inspector_server.CallTargetAttachedToTarget(mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.pause", testing::_)).WillOnce(simple_handler);
    inspector_server.OnCallDebuggerPause([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), mthread.GetId());
        handler_called = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerRemoveBreakpoint)
{
    size_t break_id = 14;

    inspector_server.CallTargetAttachedToTarget(mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.removeBreakpoint", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder res;
            JsonObject empty;
            handler(session_id, res, empty);
            ASSERT_THAT(JsonObject(std::move(res).Build()), JsonProperties());
            ASSERT_FALSE(handler_called);
        });

    auto breaks = [break_id](PtThread thread, BreakpointId bid) {
        ASSERT_EQ(bid, BreakpointId(break_id));
        ASSERT_EQ(thread.GetId(), mthread.GetId());
        handler_called = true;
    };
    inspector_server.OnCallDebuggerRemoveBreakpoint(std::move(breaks));

    EXPECT_CALL(server, OnCallMock("Debugger.removeBreakpoint", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder res;
            JsonObjectBuilder params;
            params.AddProperty("breakpointId", std::to_string(break_id));
            handler(session_id, res, JsonObject(std::move(params).Build()));
            ASSERT_THAT(JsonObject(std::move(res).Build()), JsonProperties());
            ASSERT_TRUE(handler_called);
        });

    inspector_server.OnCallDebuggerRemoveBreakpoint(std::move(breaks));
}

TEST_F(ServerTest, OnCallDebuggerRestartFrame)
{
    size_t fid = 5;

    inspector_server.CallTargetAttachedToTarget(mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.restartFrame", testing::_)).WillOnce([&](testing::Unused, auto handler) {
        std::vector<testing::Matcher<JsonObject::JsonObjPointer>> callFrames;
        JsonObjectBuilder res;
        JsonObjectBuilder params;
        handler(session_id, res, JsonObject(std::move(params).Build()));
        ASSERT_THAT(JsonObject(std::move(res).Build()), JsonProperties());
        ASSERT_FALSE(handler_called);
    });

    inspector_server.OnCallDebuggerRestartFrame([&](auto, auto) { handler_called = true; });

    EXPECT_CALL(server, OnCallMock("Debugger.restartFrame", testing::_)).WillOnce([&](testing::Unused, auto handler) {
        std::vector<testing::Matcher<JsonObject::JsonObjPointer>> callFrames;
        JsonObjectBuilder res;
        JsonObjectBuilder params;
        params.AddProperty("callFrameId", std::to_string(fid));
        handler(session_id, res, JsonObject(std::move(params).Build()));
        ASSERT_THAT(JsonObject(std::move(res).Build()),
                    JsonProperties(JsonProperty<JsonObject::ArrayT> {"callFrames", JsonElementsAreArray(callFrames)}));
        ASSERT_TRUE(handler_called);
    });

    inspector_server.OnCallDebuggerRestartFrame([&](PtThread thread, FrameId id) {
        ASSERT_EQ(id, FrameId(fid));
        ASSERT_EQ(thread.GetId(), mthread.GetId());
        handler_called = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerResume)
{
    inspector_server.CallTargetAttachedToTarget(mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.resume", testing::_)).WillOnce(simple_handler);
    inspector_server.OnCallDebuggerResume([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), mthread.GetId());
        handler_called = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerGetPossibleBreakpoints)
{
    auto script_id = 0;
    size_t start = 5;
    size_t end = 5;

    auto func = [&](auto &handler) {
        auto scope_chain = std::vector {Scope(Scope::Type::LOCAL, RemoteObject::Number(72))};
        handler(FrameId(0), std::to_string(0), source_file, 0, scope_chain);
    };

    inspector_server.CallTargetAttachedToTarget(mthread);
    inspector_server.CallDebuggerPaused(mthread, {}, {}, func);

    EXPECT_CALL(server, OnCallMock("Debugger.getPossibleBreakpoints", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder res;
            JsonObjectBuilder params;
            params.AddProperty("start", Location(script_id, start).ToJson());
            params.AddProperty("end", Location(script_id, end).ToJson());
            params.AddProperty("restrictToFunction", true);
            handler(session_id, res, JsonObject(std::move(params).Build()));
            std::vector<testing::Matcher<JsonObject::JsonObjPointer>> locations;
            for (auto i = start; i < end; i++) {
                locations.push_back(testing::Pointee(
                    JsonProperties(JsonProperty<JsonObject::StringT> {"scriptId", std::to_string(script_id)},
                                   JsonProperty<JsonObject::NumT> {"lineNumber", i})));
            }
            ASSERT_THAT(JsonObject(std::move(res).Build()), JsonProperties(JsonProperty<JsonObject::ArrayT> {
                                                                "locations", JsonElementsAreArray(locations)}));
        });
    auto get_lines_true = [](std::string_view source, size_t start_line, size_t end_line, bool param) {
        std::set<size_t> result;
        if ((source == source_file) && param) {
            for (auto i = start_line; i < end_line; i++) {
                result.insert(i);
            }
        }
        return result;
    };
    inspector_server.OnCallDebuggerGetPossibleBreakpoints(get_lines_true);

    EXPECT_CALL(server, OnCallMock("Debugger.getPossibleBreakpoints", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder res;
            JsonObjectBuilder params;
            params.AddProperty("start", Location(script_id, start).ToJson());
            params.AddProperty("end", Location(script_id, end).ToJson());
            params.AddProperty("restrictToFunction", false);
            handler(session_id, res, JsonObject(std::move(params).Build()));
            std::vector<testing::Matcher<JsonObject::JsonObjPointer>> locations;
            for (auto i = start; i < end; i++) {
                locations.push_back(testing::Pointee(
                    JsonProperties(JsonProperty<JsonObject::StringT> {"scriptId", std::to_string(script_id)},
                                   JsonProperty<JsonObject::NumT> {"lineNumber", i})));
            }
            ASSERT_THAT(JsonObject(std::move(res).Build()), JsonProperties(JsonProperty<JsonObject::ArrayT> {
                                                                "locations", JsonElementsAreArray(locations)}));
        });
    auto get_lines_false = [](std::string_view source, size_t start_line, size_t end_line, bool param) {
        std::set<size_t> result;
        if ((source == source_file) && !param) {
            for (auto i = start_line; i < end_line; i++) {
                result.insert(i);
            }
        }
        return result;
    };
    inspector_server.OnCallDebuggerGetPossibleBreakpoints(get_lines_false);

    EXPECT_CALL(server, OnCallMock("Debugger.getPossibleBreakpoints", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder res;
            JsonObjectBuilder params;
            params.AddProperty("start", Location(script_id, start).ToJson());
            params.AddProperty("end", Location(script_id + 1, end).ToJson());
            params.AddProperty("restrictToFunction", false);
            handler(session_id, res, JsonObject(std::move(params).Build()));
            std::vector<testing::Matcher<JsonObject::JsonObjPointer>> locations;
            ASSERT_THAT(JsonObject(std::move(res).Build()), JsonProperties());
        });
    inspector_server.OnCallDebuggerGetPossibleBreakpoints(get_lines_false);
}

TEST_F(ServerTest, OnCallDebuggerGetScriptSource)
{
    auto script_id = 0;

    auto func = [&](auto &handler) {
        auto scope_chain = std::vector {Scope(Scope::Type::LOCAL, RemoteObject::Number(72))};
        handler(FrameId(0), std::to_string(0), source_file, 0, scope_chain);
    };

    EXPECT_CALL(server, CallMock(session_id, "Debugger.paused", testing::_))
        .WillOnce([&](testing::Unused, testing::Unused, auto s) { ToObject(std::move(s)); });
    EXPECT_CALL(server, CallMock(session_id, "Debugger.scriptParsed", testing::_)).Times(1);

    inspector_server.CallTargetAttachedToTarget(mthread);
    inspector_server.CallDebuggerPaused(mthread, {}, {}, func);

    EXPECT_CALL(server, OnCallMock("Debugger.getScriptSource", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder res;
            JsonObjectBuilder params;
            params.AddProperty("scriptId", std::to_string(script_id));
            handler(session_id, res, JsonObject(std::move(params).Build()));
            ASSERT_THAT(JsonObject(std::move(res).Build()),
                        JsonProperties(JsonProperty<JsonObject::StringT> {"scriptSource", source_file}));
        });
    inspector_server.OnCallDebuggerGetScriptSource([](auto source) {
        std::string s(source);
        return s;
    });

    EXPECT_CALL(server, OnCallMock("Debugger.getScriptSource", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder res;
            JsonObject empty;
            handler(session_id, res, empty);
            ASSERT_THAT(JsonObject(std::move(res).Build()), JsonProperties());
        });
    inspector_server.OnCallDebuggerGetScriptSource([](auto) { return "a"; });
}

TEST_F(ServerTest, OnCallDebuggerStepOut)
{
    inspector_server.CallTargetAttachedToTarget(mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.stepOut", testing::_)).WillOnce(simple_handler);

    inspector_server.OnCallDebuggerStepOut([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), mthread.GetId());
        handler_called = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerStepInto)
{
    inspector_server.CallTargetAttachedToTarget(mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.stepInto", testing::_)).WillOnce(simple_handler);

    inspector_server.OnCallDebuggerStepInto([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), mthread.GetId());
        handler_called = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerStepOver)
{
    inspector_server.CallTargetAttachedToTarget(mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.stepOver", testing::_)).WillOnce(simple_handler);

    inspector_server.OnCallDebuggerStepOver([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), mthread.GetId());
        handler_called = true;
    });
}

std::optional<BreakpointId> handlerForSetBreak([[maybe_unused]] PtThread thread,
                                               [[maybe_unused]] const std::function<bool(std::string_view)> &comp,
                                               size_t line, [[maybe_unused]] std::set<std::string_view> &sources)
{
    sources.insert("source");
    return BreakpointId(line);
}

std::optional<BreakpointId> handlerForSetBreakEmpty([[maybe_unused]] PtThread thread,
                                                    [[maybe_unused]] const std::function<bool(std::string_view)> &comp,
                                                    [[maybe_unused]] size_t line,
                                                    [[maybe_unused]] std::set<std::string_view> &sources)
{
    sources.insert("source");
    return {};
}

TEST_F(ServerTest, OnCallDebuggerSetBreakpoint)
{
    auto script_id = 0;
    size_t start = 5;

    auto func = [&](auto &handler) {
        auto scope_chain = std::vector {Scope(Scope::Type::LOCAL, RemoteObject::Number(72))};
        handler(FrameId(0), std::to_string(0), source_file, 0, scope_chain);
    };

    inspector_server.CallTargetAttachedToTarget(mthread);
    inspector_server.CallDebuggerPaused(mthread, {}, {}, func);

    EXPECT_CALL(server, OnCallMock("Debugger.setBreakpoint", testing::_)).WillOnce([&](testing::Unused, auto handler) {
        JsonObjectBuilder res;
        JsonObjectBuilder params;
        params.AddProperty("location", Location(script_id, start).ToJson());
        handler(session_id, res, JsonObject(std::move(params).Build()));
        ASSERT_THAT(JsonObject(std::move(res).Build()),
                    JsonProperties(JsonProperty<JsonObject::StringT> {"breakpointId", std::to_string(start)},
                                   JsonProperty<JsonObject::JsonObjPointer> {
                                       "actualLocation",
                                       testing::Pointee(JsonProperties(
                                           JsonProperty<JsonObject::StringT> {"scriptId", std::to_string(script_id)},
                                           JsonProperty<JsonObject::NumT> {"lineNumber", start - 1}))}));
    });

    inspector_server.OnCallDebuggerSetBreakpoint(handlerForSetBreak);

    EXPECT_CALL(server, OnCallMock("Debugger.setBreakpoint", testing::_)).WillOnce([&](testing::Unused, auto handler) {
        JsonObjectBuilder res;
        JsonObject empty;
        handler(session_id, res, empty);
        ASSERT_THAT(JsonObject(std::move(res).Build()), JsonProperties());
    });

    inspector_server.OnCallDebuggerSetBreakpoint(handlerForSetBreak);

    EXPECT_CALL(server, OnCallMock("Debugger.setBreakpoint", testing::_)).WillOnce([&](testing::Unused, auto handler) {
        JsonObjectBuilder res;
        JsonObjectBuilder params;
        params.AddProperty("location", Location(script_id, start).ToJson());
        handler(session_id, res, JsonObject(std::move(params).Build()));
        ASSERT_THAT(JsonObject(std::move(res).Build()), JsonProperties());
    });

    inspector_server.OnCallDebuggerSetBreakpoint(handlerForSetBreakEmpty);
}

TEST_F(ServerTest, OnCallDebuggerSetBreakpointByUrl)
{
    auto script_id = 0;
    size_t start = 5;

    inspector_server.CallTargetAttachedToTarget(mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.setBreakpointByUrl", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder res;
            JsonObjectBuilder params;
            params.AddProperty("lineNumber", start);
            handler(session_id, res, JsonObject(std::move(params).Build()));
            ASSERT_THAT(JsonObject(std::move(res).Build()), JsonProperties());
        });

    inspector_server.OnCallDebuggerSetBreakpointByUrl(handlerForSetBreak);

    EXPECT_CALL(server, OnCallMock("Debugger.setBreakpointByUrl", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder res;
            JsonObjectBuilder params;
            params.AddProperty("lineNumber", start);
            params.AddProperty("url", "file://source");
            handler(session_id, res, JsonObject(std::move(params).Build()));
            std::vector<testing::Matcher<JsonObject::JsonObjPointer>> locations;
            locations.push_back(testing::Pointee(
                JsonProperties(JsonProperty<JsonObject::StringT> {"scriptId", std::to_string(script_id)},
                               JsonProperty<JsonObject::NumT> {"lineNumber", start})));

            ASSERT_THAT(
                JsonObject(std::move(res).Build()),
                JsonProperties(JsonProperty<JsonObject::StringT> {"breakpointId", std::to_string(start + 1)},
                               JsonProperty<JsonObject::ArrayT> {"locations", JsonElementsAreArray(locations)}));
        });

    inspector_server.OnCallDebuggerSetBreakpointByUrl(handlerForSetBreak);
}

TEST_F(ServerTest, OnCallDebuggerSetBreakpointsActive)
{
    inspector_server.CallTargetAttachedToTarget(mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.setBreakpointsActive", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder res;
            JsonObject empty;
            handler(session_id, res, empty);
            ASSERT_THAT(JsonObject(std::move(res).Build()), JsonProperties());
            ASSERT_FALSE(handler_called);
        });

    inspector_server.OnCallDebuggerSetBreakpointsActive([](auto thread, auto value) {
        ASSERT_EQ(thread.GetId(), mthread.GetId());
        ASSERT_FALSE(value);
        handler_called = true;
    });

    EXPECT_CALL(server, OnCallMock("Debugger.setBreakpointsActive", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder res;
            JsonObjectBuilder params;
            params.AddProperty("active", true);
            handler(session_id, res, JsonObject(std::move(params).Build()));
            ASSERT_THAT(JsonObject(std::move(res).Build()), JsonProperties());
            ASSERT_TRUE(handler_called);
            handler_called = false;
        });

    inspector_server.OnCallDebuggerSetBreakpointsActive([](auto thread, auto value) {
        ASSERT_EQ(thread.GetId(), mthread.GetId());
        ASSERT_TRUE(value);
        handler_called = true;
    });

    EXPECT_CALL(server, OnCallMock("Debugger.setBreakpointsActive", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder res;
            JsonObjectBuilder params;
            params.AddProperty("active", false);
            handler(session_id, res, JsonObject(std::move(params).Build()));
            ASSERT_THAT(JsonObject(std::move(res).Build()), JsonProperties());
            ASSERT_TRUE(handler_called);
        });

    inspector_server.OnCallDebuggerSetBreakpointsActive([](auto thread, auto value) {
        ASSERT_EQ(thread.GetId(), mthread.GetId());
        ASSERT_FALSE(value);
        handler_called = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerSetPauseOnExceptions)
{
    inspector_server.CallTargetAttachedToTarget(mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.setPauseOnExceptions", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder res;
            JsonObjectBuilder params;
            params.AddProperty("state", "none");
            handler(session_id, res, JsonObject(std::move(params).Build()));
            ASSERT_THAT(JsonObject(std::move(res).Build()), JsonProperties());
            ASSERT_TRUE(handler_called);
        });

    inspector_server.OnCallDebuggerSetPauseOnExceptions([](PtThread thread, PauseOnExceptionsState state) {
        ASSERT_EQ(thread.GetId(), mthread.GetId());
        ASSERT_EQ(PauseOnExceptionsState::NONE, state);
        handler_called = true;
    });
}

TEST_F(ServerTest, OnCallRuntimeEnable)
{
    inspector_server.CallTargetAttachedToTarget(mthread);

    EXPECT_CALL(server, OnCallMock("Runtime.enable", testing::_)).WillOnce([&](testing::Unused, auto handler) {
        JsonObjectBuilder res;
        JsonObject empty;
        handler(session_id, res, empty);
        ASSERT_THAT(JsonObject(std::move(res).Build()), JsonProperties());
        ASSERT_TRUE(handler_called);
    });
    inspector_server.OnCallRuntimeEnable([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), mthread.GetId());
        handler_called = true;
    });
}

TEST_F(ServerTest, OnCallRuntimeGetProperties)
{
    auto object_id = 6;
    auto preview = true;

    inspector_server.CallTargetAttachedToTarget(mthread);

    EXPECT_CALL(server, OnCallMock("Runtime.getProperties", testing::_)).WillOnce([&](testing::Unused, auto handler) {
        JsonObjectBuilder res;
        JsonObjectBuilder params;
        params.AddProperty("objectId", std::to_string(object_id));
        params.AddProperty("generatePreview", preview);
        handler(session_id, res, JsonObject(std::move(params).Build()));

        std::vector<testing::Matcher<JsonObject::JsonObjPointer>> result;
        result.push_back(testing::Pointee(JsonProperties(
            JsonProperty<JsonObject::StringT> {"name", "object"},
            JsonProperty<JsonObject::JsonObjPointer> {
                "value", testing::Pointee(JsonProperties(JsonProperty<JsonObject::NumT> {"value", object_id},
                                                         JsonProperty<JsonObject::StringT> {"type", "number"}))},
            JsonProperty<JsonObject::BoolT> {"writable", testing::_},
            JsonProperty<JsonObject::BoolT> {"configurable", testing::_},
            JsonProperty<JsonObject::BoolT> {"enumerable", testing::_})));
        result.push_back(testing::Pointee(JsonProperties(
            JsonProperty<JsonObject::StringT> {"name", "preview"},
            JsonProperty<JsonObject::JsonObjPointer> {
                "value", testing::Pointee(JsonProperties(JsonProperty<JsonObject::BoolT> {"value", preview},
                                                         JsonProperty<JsonObject::StringT> {"type", "boolean"}))},
            JsonProperty<JsonObject::BoolT> {"writable", testing::_},
            JsonProperty<JsonObject::BoolT> {"configurable", testing::_},
            JsonProperty<JsonObject::BoolT> {"enumerable", testing::_})));
        result.push_back(testing::Pointee(JsonProperties(
            JsonProperty<JsonObject::StringT> {"name", "threadId"},
            JsonProperty<JsonObject::JsonObjPointer> {
                "value", testing::Pointee(JsonProperties(JsonProperty<JsonObject::NumT> {"value", mthread.GetId()},
                                                         JsonProperty<JsonObject::StringT> {"type", "number"}))},
            JsonProperty<JsonObject::BoolT> {"writable", testing::_},
            JsonProperty<JsonObject::BoolT> {"configurable", testing::_},
            JsonProperty<JsonObject::BoolT> {"enumerable", testing::_})));

        ASSERT_THAT(JsonObject(std::move(res).Build()),
                    JsonProperties(JsonProperty<JsonObject::ArrayT> {"result", JsonElementsAreArray(result)}));
    });

    auto getProperties = [](PtThread thread, RemoteObjectId id, bool need_preview) {
        std::vector<PropertyDescriptor> res;
        res.push_back(PropertyDescriptor("object", RemoteObject::Number(id)));
        res.push_back(PropertyDescriptor("preview", RemoteObject::Boolean(need_preview)));
        res.push_back(PropertyDescriptor("threadId", RemoteObject::Number(thread.GetId())));
        return res;
    };

    inspector_server.OnCallRuntimeGetProperties(getProperties);
}

TEST_F(ServerTest, OnCallRuntimeRunIfWaitingForDebugger)
{
    inspector_server.CallTargetAttachedToTarget(mthread);

    EXPECT_CALL(server, OnCallMock("Runtime.runIfWaitingForDebugger", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder res;
            JsonObject empty;
            handler(session_id, res, empty);
            ASSERT_THAT(JsonObject(std::move(res).Build()), JsonProperties());
            ASSERT_TRUE(handler_called);
        });

    inspector_server.OnCallRuntimeRunIfWaitingForDebugger([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), mthread.GetId());
        handler_called = true;
    });
}

}  // namespace panda::tooling::inspector::test

// NOLINTEND
