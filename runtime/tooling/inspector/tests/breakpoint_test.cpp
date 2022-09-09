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
#include "expect_pauses.h"
#include "inspector_test_base.h"
#include "instruction_pointer.h"
#include "json_object_matcher.h"
#include "test_frame.h"
#include "test_method.h"

#include "utils/json_builder.h"
#include "utils/json_parser.h"
#include "utils/utf.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <cstddef>
#include <optional>
#include <string>
#include <utility>

using testing::_;
using testing::Matcher;

namespace panda::tooling::inspector::test {
class BreakpointTest : public InspectorTestBase {
protected:
    void SetUpSourceFiles() override
    {
        auto klass = LoadSourceFile(R"(
            .function void func() {
                nop

                return
            }

            .function void main() {
                call func
                call func
                return
            }
        )");

        mainFrame_.SetMethod(klass->GetDirectMethod(utf::CStringAsMutf8("main")));
        func_.Set(klass->GetDirectMethod(utf::CStringAsMutf8("func")));
    }

    void SetUp() override
    {
        InspectorTestBase::SetUp();

        client_.OnCall("Debugger.scriptParsed", [this](const JsonObject &script) {
            auto scriptId = script.GetValue<JsonObject::StringT>("scriptId");
            ASSERT_NE(scriptId, nullptr) << "No 'scriptId' property";
            scriptId_ = *scriptId;

            auto url = script.GetValue<JsonObject::StringT>("url");
            ASSERT_NE(url, nullptr) << "No 'url' property";
            url_ = *url;
        });

        main_.Resume();
        (server_ + client_).Poll();
        EXPECT_FALSE(scriptId_.empty());
    }

    void TearDown() override
    {
        main_.Finish();
        InspectorTestBase::TearDown();
    }

    template <size_t... Breakpoint>
    void CheckPossibleBreakpoints(size_t startLine, std::optional<size_t> endLine,
                                  std::optional<bool> restrictToFunction,
                                  std::index_sequence<Breakpoint...> /* breakpoints */);

    std::string scriptId_;
    std::string url_;
    TestFrame mainFrame_ {debugger_};
    InstructionPointer main_ {mainFrame_, client_, debugger_};
    TestMethod func_ {debugger_, client_};
};

template <size_t... Breakpoint>
void BreakpointTest::CheckPossibleBreakpoints(size_t startLine, std::optional<size_t> endLine,
                                              std::optional<bool> restrictToFunction,
                                              std::index_sequence<Breakpoint...> /* breakpoints */)
{
    CallSync(
        "Debugger.getPossibleBreakpoints",
        [this, startLine, endLine, restrictToFunction](JsonObjectBuilder &params) {
            params.AddProperty("start", [this, startLine](JsonObjectBuilder &start) {
                start.AddProperty("scriptId", scriptId_);
                start.AddProperty("lineNumber", startLine);
            });
            if (endLine) {
                params.AddProperty("end", [this, endLine](JsonObjectBuilder &end) {
                    end.AddProperty("scriptId", scriptId_);
                    end.AddProperty("lineNumber", *endLine);
                });
            }
            if (restrictToFunction) {
                params.AddProperty("restrictToFunction", *restrictToFunction);
            }
        },
        [this](const JsonObject &result) {
            (void)this;
            auto resultMatcher = JsonProperties(JsonProperty<JsonObject::ArrayT> {
                "array", JsonElements(Matcher<JsonObject::JsonObjPointer> {
                             Pointee(JsonProperties(JsonProperty<JsonObject::StringT> {"scriptId", scriptId_},
                                                    JsonProperty<JsonObject::NumT> {"lineNumber", Breakpoint}))}...)});
            EXPECT_THAT(result, resultMatcher);
        });  // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks) due to bug in clang-tidy #3553 (gtest repo)
}

TEST_F(BreakpointTest, GetsPossibleBreakpoints)
{
    CheckPossibleBreakpoints(4, 10, std::nullopt, std::index_sequence<4, 8, 9> {});
    CheckPossibleBreakpoints(9, std::nullopt, std::nullopt, std::index_sequence<9, 10> {});
    CheckPossibleBreakpoints(4, 9, true, std::index_sequence<4> {});
    CheckPossibleBreakpoints(3, std::nullopt, true, std::index_sequence<4> {});
    CheckPossibleBreakpoints(6, 10, true, std::index_sequence<> {});
    CheckPossibleBreakpoints(0, std::nullopt, true, std::index_sequence<> {});
}

// When stopped at a breakpoint while doing a step and resumed,
// the unfinished step command should be canceled.
TEST_F(BreakpointTest, InterruptsStep)
{
    CallSync("Debugger.setBreakpoint", [&](auto &params) {
        params.AddProperty("location", [&](JsonObjectBuilder &location) {
            location.AddProperty("scriptId", scriptId_);
            location.AddProperty("lineNumber", 2);
        });
    });

    CallSync("Debugger.setBreakpoint", [&](auto &params) {
        params.AddProperty("location", [&](JsonObjectBuilder &location) {
            location.AddProperty("scriptId", scriptId_);
            location.AddProperty("lineNumber", 9);
        });
    });

    ExpectPauses(client_, {
                              Breakpoint(CallFrame(func_, 2), CallFrame(main_.GetMethod(), 8)),
                              Breakpoint(CallFrame(main_.GetMethod(), 9)),
                              Breakpoint(CallFrame(func_, 2), CallFrame(main_.GetMethod(), 9)),
                          });

    func_.Call([](auto &func) { func.Resume(); });
    main_.StepOver();
    func_.Call([](auto &func) { func.Resume(); });
    main_.Finish();
}

TEST_F(BreakpointTest, RemovesBreakpoint)
{
    auto breakpointId = CallSync(
        "Debugger.setBreakpoint",
        [&this](auto &params) {
            params.AddProperty("location", [&this](JsonObjectBuilder &location) {
                location.AddProperty("scriptId", scriptId_);
                location.AddProperty("lineNumber", 2);
            });
        },
        [](const JsonObject &result) {
            auto id = result.GetValue<JsonObject::StringT>("breakpointId");
            return id != nullptr ? std::make_optional(*id) : std::nullopt;
        });
    ASSERT_TRUE(breakpointId.has_value());

    ExpectPauses(client_, {
                              Breakpoint(CallFrame(func_, 2), CallFrame(main_.GetMethod(), 8)),
                          });

    func_.Call([](auto &func) { func.Resume(); });
    main_.Step();

    CallSync(
        "Debugger.removeBreakpoint", [&](auto &params) { params.AddProperty("breakpointId", *breakpointId); },
        // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks) due to bug in clang-tidy #3553 (gtest repo)
        [](auto &result) { EXPECT_THAT(result, JsonProperties()); });

    func_.Call([](auto &func) { func.Resume(); });
    main_.Finish();
}

TEST_F(BreakpointTest, SetsBreakpoint)
{
    CallSync(
        "Debugger.setBreakpoint",
        [&](auto &params) {
            params.AddProperty("location", [&](JsonObjectBuilder &location) {
                location.AddProperty("scriptId", scriptId_);
                location.AddProperty("lineNumber", 2);
            });
        },
        [&](auto &result) {
            EXPECT_THAT(
                result,
                JsonProperties(JsonProperty<JsonObject::StringT> {"breakpointId", _},
                               JsonProperty<JsonObject::JsonObjPointer> {
                                   "actualLocation",
                                   Pointee(JsonProperties(JsonProperty<JsonObject::StringT> {"scriptId", scriptId_},
                                                          JsonProperty<JsonObject::NumT> {"lineNumber", 2}))}));
        });

    ExpectPauses(client_, {
                              Breakpoint(CallFrame(func_, 2), CallFrame(main_.GetMethod(), 8)),
                              Breakpoint(CallFrame(func_, 2), CallFrame(main_.GetMethod(), 9)),
                          });

    func_.Call([](auto &func) { func.Resume(); });
    main_.Step();
    func_.Call([](auto &func) { func.Resume(); });
    main_.Finish();
}

TEST_F(BreakpointTest, SetsBreakpointByUrl)
{
    CallSync(
        "Debugger.setBreakpointByUrl",
        [&](auto &params) {
            params.AddProperty("lineNumber", 2);
            params.AddProperty("url", url_);
        },
        [&](auto &result) {
            EXPECT_THAT(result,
                        JsonProperties(JsonProperty<JsonObject::StringT> {"breakpointId", _},
                                       JsonProperty<JsonObject::ArrayT> {
                                           "locations",
                                           JsonElements(Matcher<JsonObject::JsonObjPointer> {Pointee(
                                               JsonProperties(JsonProperty<JsonObject::StringT> {"scriptId", scriptId_},
                                                              JsonProperty<JsonObject::NumT> {"lineNumber", 2}))})}));
        });

    ExpectPauses(client_, {
                              Breakpoint(CallFrame(func_, 2), CallFrame(main_.GetMethod(), 8)),
                              Breakpoint(CallFrame(func_, 2), CallFrame(main_.GetMethod(), 9)),
                          });

    func_.Call([](auto &func) { func.Resume(); });
    main_.Step();
    func_.Call([](auto &func) { func.Resume(); });
    main_.Finish();
}

TEST_F(BreakpointTest, SetsBreakpointByUrlRegex)
{
    CallSync(
        "Debugger.setBreakpointByUrl",
        [](auto &params) {
            params.AddProperty("lineNumber", 2);
            params.AddProperty("urlRegex", ".*");
        },
        [&](auto &result) {
            EXPECT_THAT(result,
                        JsonProperties(JsonProperty<JsonObject::StringT> {"breakpointId", _},
                                       JsonProperty<JsonObject::ArrayT> {
                                           "locations",
                                           JsonElements(Matcher<JsonObject::JsonObjPointer> {Pointee(
                                               JsonProperties(JsonProperty<JsonObject::StringT> {"scriptId", scriptId_},
                                                              JsonProperty<JsonObject::NumT> {"lineNumber", 2}))})}));
        });

    ExpectPauses(client_, {
                              Breakpoint(CallFrame(func_, 2), CallFrame(main_.GetMethod(), 8)),
                              Breakpoint(CallFrame(func_, 2), CallFrame(main_.GetMethod(), 9)),
                          });

    func_.Call([](auto &func) { func.Resume(); });
    main_.Step();
    func_.Call([](auto &func) { func.Resume(); });
    main_.Finish();
}

TEST_F(BreakpointTest, SetsBreakpointsActive)
{
    CallSync("Debugger.setBreakpoint", [&](auto &params) {
        params.AddProperty("location", [&](JsonObjectBuilder &location) {
            location.AddProperty("scriptId", scriptId_);
            location.AddProperty("lineNumber", 2);
        });
    });

    CallSync(
        "Debugger.setBreakpointsActive", [](auto &params) { params.AddProperty("active", false); },
        // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks) due to bug in clang-tidy #3553 (gtest repo)
        [](auto &result) { EXPECT_THAT(result, JsonProperties()); });

    ExpectPauses(client_, {
                              Breakpoint(CallFrame(func_, 2), CallFrame(main_.GetMethod(), 9)),
                          });

    func_.Call([](auto &func) { func.Resume(); });
    main_.Step();

    CallSync(
        "Debugger.setBreakpointsActive", [](auto &params) { params.AddProperty("active", true); },
        // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks) due to bug in clang-tidy #3553 (gtest repo)
        [](auto &result) { EXPECT_THAT(result, JsonProperties()); });

    func_.Call([](auto &func) { func.Resume(); });
    main_.Finish();
}
}  // namespace panda::tooling::inspector::test
