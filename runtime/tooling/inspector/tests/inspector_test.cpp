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

#include "combined_event_loop.h"
#include "common.h"
#include "inspector_test_base.h"
#include "json_object_matcher.h"

#include "tooling/pt_location.h"
#include "tooling/pt_thread.h"
#include "utils/json_builder.h"
#include "utils/json_parser.h"
#include "utils/logger.h"
#include "utils/utf.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <utility>

using testing::_;
using testing::HasSubstr;
using testing::InSequence;
using testing::Matcher;
using testing::MockFunction;

namespace panda::tooling::inspector::test {
class InspectorTest : public InspectorTestBase {
protected:
    bool AttachDebugger() const override
    {
        return false;
    }

    template <typename... Arg>
    void Call(const char *method, Arg &&... arg)
    {
        auto savedPrefix = logger_.SetPrefix(client_.GetName());
        client_.Call(method, std::forward<Arg>(arg)...);
        logger_.SetPrefix(savedPrefix);
    }

    template <typename... Arg>
    void OnCall(const char *method, Arg &&... arg)
    {
        client_.OnCall(method, std::forward<Arg>(arg)...);
    }

    template <typename... MethodName>
    void ExpectUnsupportedMethods(MethodName... method)
    {
        EXPECT_CALL(logger_, LogLineInternal(Logger::Level::WARNING, Logger::Component::DEBUGGER,
                                             HasSubstr("Unsupported method"))).Times(0);

        (EXPECT_CALL(logger_, LogLineInternal(Logger::Level::WARNING, Logger::Component::DEBUGGER,
                                              AllOf(HasSubstr("Unsupported method"), HasSubstr(method)))), ...);
    }
};

namespace {
class Callback {
public:
    template <typename... PropertyType>
    explicit Callback(MockFunction<void()> &checkpoint, JsonProperty<PropertyType>... property)
        : checkpoint_(&checkpoint), resultMatcher_(JsonProperties(std::move(property)...))
    {
    }

    template <typename... PropertyType>
    explicit Callback(JsonProperty<PropertyType>... property)
        : checkpoint_(nullptr), resultMatcher_(JsonProperties(std::move(property)...))
    {
    }

    void operator()(const JsonObject &result)
    {
        EXPECT_THAT(result, resultMatcher_);

        if (checkpoint_ != nullptr) {
            checkpoint_->Call();
        }
    }

private:
    MockFunction<void()> *checkpoint_;
    Matcher<const JsonObject &> resultMatcher_;
};
}  // namespace

TEST_F(InspectorTest, InitialSequence)
{
    ExpectUnsupportedMethods("Profiler.enable", "Debugger.setPauseOnExceptions", "Debugger.setAsyncCallStackDepth",
                             "Runtime.getIsolateId", "Debugger.setBlackboxPatterns");

    // NOLINTNEXTLINE(readability-isolate-declaration)
    MockFunction<void()> executionContextCreated;
    MockFunction<void()> runtimeEnabled;
    MockFunction<void()> debuggerEnabled;
    MockFunction<void()> debuggerResumed;
    MockFunction<void()> runIfWaiting;
    InSequence seq;
    EXPECT_CALL(executionContextCreated, Call);
    EXPECT_CALL(runtimeEnabled, Call);
    EXPECT_CALL(debuggerEnabled, Call);
    EXPECT_CALL(debuggerResumed, Call);
    EXPECT_CALL(runIfWaiting, Call);

    Call("Profiler.enable", Callback());
    Call("Runtime.enable", Callback(runtimeEnabled));
    OnCall("Runtime.executionContextCreated",
           Callback(executionContextCreated,
                    JsonProperty<JsonObject::JsonObjPointer> {
                        "context", Pointee(JsonProperties(JsonProperty<JsonObject::NumT> {"id", IsInteger()},
                                                          JsonProperty<JsonObject::StringT> {"origin", _},
                                                          JsonProperty<JsonObject::StringT> {"name", _}))}));
    Call(
        "Debugger.enable", [](auto &params) { params.AddProperty("maxScriptsCacheSize", 10000000); },
        Callback(debuggerEnabled, JsonProperty<JsonObject::StringT> {"debuggerId", _}));
    Call(
        "Debugger.setPauseOnExceptions", [](auto &params) { params.AddProperty("state", "none"); }, Callback());
    Call(
        "Debugger.setAsyncCallStackDepth", [](auto &params) { params.AddProperty("maxDepth", 32); }, Callback());
    Call("Runtime.getIsolateId", Callback(JsonProperty<JsonObject::StringT> {"id", _}));
    Call(
        "Debugger.setBlackboxPatterns",
        [](auto &params) { params.AddProperty("patterns", [](JsonArrayBuilder & /* patterns */) {}); }, Callback());
    Call("Runtime.runIfWaitingForDebugger", Callback(runIfWaiting));
    OnCall("Debugger.resumed", Callback(debuggerResumed));

    debugger_.GetHooks().VmInitialization(PtThread::NONE);

    (server_ + client_).Poll();
}

TEST_F(InspectorTest, BreaksOnStart)
{
    // NOLINTNEXTLINE(readability-isolate-declaration)
    MockFunction<void()> debuggerPaused;
    MockFunction<void()> debuggerResumed;
    MockFunction<void()> debuggerResumeRepliedTo;
    InSequence seq;
    EXPECT_CALL(debuggerResumed, Call);
    EXPECT_CALL(debuggerPaused, Call);
    EXPECT_CALL(debuggerResumed, Call);
    EXPECT_CALL(debuggerResumeRepliedTo, Call);

    Call("Runtime.runIfWaitingForDebugger");
    OnCall("Debugger.resumed", Callback(debuggerResumed));
    debugger_.GetHooks().VmInitialization(PtThread::NONE);

    Call(
        "Debugger.resume", [](auto & /* params */) {}, Callback(debuggerResumeRepliedTo));
    OnCall("Debugger.paused", Callback(debuggerPaused, JsonProperty<JsonObject::ArrayT> {"callFrames", _},
                                       JsonProperty<JsonObject::StringT> {"reason", "Break on start"}));
    debugger_.GetHooks().SingleStep(
        PtThread::NONE, LoadSourceFile(".function void foo() {}")->GetDirectMethod(utf::CStringAsMutf8("foo")),
        PtLocation("", {}, 0));

    (server_ + client_).Poll();
}
}  // namespace panda::tooling::inspector::test
