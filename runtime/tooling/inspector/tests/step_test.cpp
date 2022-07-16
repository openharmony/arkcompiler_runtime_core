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
#include "test_method.h"

#include "utils/json_parser.h"
#include "utils/utf.h"

#include "gtest/gtest.h"

#include <string>

namespace panda::tooling::inspector::test {
class StepTest : public InspectorTestBase {
protected:
    void SetUpSourceFiles() override
    {
        auto klass = LoadSourceFile(R"(
            .function void main() {
                call foo
                call foo
                return
            }

            .function void foo() {
                nop
                call bar
                return
            }

            .function void bar() {
                return
            }
        )");

        main_.Set(klass->GetDirectMethod(utf::CStringAsMutf8("main")));
        foo_.Set(klass->GetDirectMethod(utf::CStringAsMutf8("foo")));
        bar_.Set(klass->GetDirectMethod(utf::CStringAsMutf8("bar")));
    }

    TestMethod main_ {debugger_, client_};
    TestMethod foo_ {debugger_, client_};
    TestMethod bar_ {debugger_, client_};
};

TEST_F(StepTest, ContinuesToLocation)
{
    std::string scriptId;

    client_.OnCall("Debugger.scriptParsed", [&](const JsonObject &script) {
        auto scriptIdPtr = script.GetValue<JsonObject::StringT>("scriptId");
        ASSERT_NE(scriptIdPtr, nullptr) << "No 'scriptId' property";
        scriptId = *scriptIdPtr;
    });

    ExpectPauses(client_, {
                              BreakOnStart(CallFrame(main_, 2)),
                              Step(CallFrame(foo_, 8), CallFrame(main_, 2)),
                              Step(CallFrame(bar_, 14), CallFrame(foo_, 9), CallFrame(main_, 2)),
                              Step(CallFrame(foo_, 8), CallFrame(main_, 3)),
                          });

    main_.Call([&](auto &main) {
        main.StepInto();

        (server_ + client_).Poll();
        ASSERT_FALSE(scriptId.empty());

        foo_.Call([&](auto &foo) {
            foo.ContinueTo(scriptId, 14);
            foo.Step();
            bar_.Call([&](auto &bar) { bar.ContinueTo(scriptId, 8); });
        });

        main.Step();
        foo_.Call([&](auto &foo) {
            foo.Resume();
            foo.Step();
            bar_.Call();
        });
    });
}

TEST_F(StepTest, StepsInto)
{
    ExpectPauses(client_, {
                              BreakOnStart(CallFrame(main_, 2)),
                              Step(CallFrame(foo_, 8), CallFrame(main_, 2)),
                              Step(CallFrame(foo_, 9), CallFrame(main_, 2)),
                              Step(CallFrame(bar_, 14), CallFrame(foo_, 9), CallFrame(main_, 2)),
                              Step(CallFrame(foo_, 10), CallFrame(main_, 2)),
                              Step(CallFrame(main_, 3)),
                              Step(CallFrame(foo_, 8), CallFrame(main_, 3)),
                              Step(CallFrame(foo_, 9), CallFrame(main_, 3)),
                              Step(CallFrame(bar_, 14), CallFrame(foo_, 9), CallFrame(main_, 3)),
                              Step(CallFrame(foo_, 10), CallFrame(main_, 3)),
                              Step(CallFrame(main_, 4)),
                          });

    main_.Call([&](auto &main) {
        for (unsigned times = 2; times-- != 0;) {
            main.StepInto();

            foo_.Call([&](auto &foo) {
                foo.StepInto();
                foo.StepInto();
                bar_.Call([](auto &bar) { bar.StepInto(); });
                foo.StepInto();
            });
        }

        main.StepInto();
    });
}

TEST_F(StepTest, StepsOut)
{
    ExpectPauses(client_, {
                              BreakOnStart(CallFrame(main_, 2)),
                              Step(CallFrame(foo_, 8), CallFrame(main_, 2)),
                              Step(CallFrame(foo_, 9), CallFrame(main_, 2)),
                              Step(CallFrame(main_, 3)),
                          });

    main_.Call([&](auto &main) {
        main.StepInto();

        foo_.Call([&](auto &foo) {
            foo.StepInto();
            foo.StepOut();
            bar_.Call();
        });

        main.StepOut();

        foo_.Call([&](auto &foo) {
            foo.Step();
            foo.Step();
            bar_.Call();
        });
    });
}

TEST_F(StepTest, StepsOver)
{
    ExpectPauses(client_, {
                              BreakOnStart(CallFrame(main_, 2)),
                              Step(CallFrame(main_, 3)),
                              Step(CallFrame(foo_, 8), CallFrame(main_, 3)),
                              Step(CallFrame(foo_, 9), CallFrame(main_, 3)),
                              Step(CallFrame(foo_, 10), CallFrame(main_, 3)),
                              Step(CallFrame(main_, 4)),
                          });

    main_.Call([&](auto &main) {
        main.StepOver();
        foo_.Call();
        main.StepInto();

        foo_.Call([&](auto &foo) {
            foo.StepOver();
            foo.StepOver();
            bar_.Call();
            foo.StepOver();
        });

        main.StepOver();
    });
}
}  // namespace panda::tooling::inspector::test
