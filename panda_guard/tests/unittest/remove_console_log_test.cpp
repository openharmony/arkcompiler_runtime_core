/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "assembler/assembly-parser.h"

#include "obfuscate/function.h"
#include "obfuscate/program.h"

using namespace testing::ext;
using namespace panda::guard;

namespace {
constexpr std::string_view CONSOLE_NAME = "console";

size_t CountConsoleLoads(const panda::pandasm::Function &func)
{
    size_t count = 0;
    for (const auto &ins : func.ins) {
        if (ins->GetOpcode() == panda::pandasm::Opcode::TRYLDGLOBALBYNAME && ins->GetId(0) == CONSOLE_NAME) {
            count++;
        }
    }
    return count;
}

bool HasOpcode(const panda::pandasm::Function &func, panda::pandasm::Opcode opcode)
{
    for (const auto &ins : func.ins) {
        if (ins->GetOpcode() == opcode) {
            return true;
        }
    }
    return false;
}

bool HasLabel(const panda::pandasm::Function &func, const std::string &name)
{
    for (const auto &ins : func.ins) {
        if (ins->IsLabel() && ins->Label() == name) {
            return true;
        }
    }
    return false;
}

bool HasJumpTo(const panda::pandasm::Function &func, const std::string &label)
{
    for (const auto &ins : func.ins) {
        if (ins->GetOpcode() == panda::pandasm::Opcode::JNEZ && ins->GetId(0) == label) {
            return true;
        }
    }
    return false;
}

panda::pandasm::Function &GetFunction(panda::pandasm::Program &program, const std::string &name)
{
    auto it = program.function_table.find(name);
    if (it == program.function_table.end()) {
        std::string keys;
        for (const auto &[key, _] : program.function_table) {
            keys += key + ",";
        }
        EXPECT_NE(it, program.function_table.end()) << "function not found: " << name << " existing: " << keys;
        static panda::pandasm::Function dummy("dummy", panda::panda_file::SourceLang::ECMASCRIPT);
        return dummy;
    }
    return it->second;
}

void RemoveConsoleLog(panda::pandasm::Program &program, const std::string &funcName)
{
    panda::guard::Program guardProgram(&program);
    panda::guard::Function guardFunction(&guardProgram, funcName, false);
    guardFunction.RemoveConsoleLog();
}

panda::pandasm::Program ParseProgram(const std::string &source)
{
    panda::pandasm::Parser parser;
    auto res = parser.Parse(source, "remove_console_log_test.pa");
    EXPECT_EQ(parser.ShowError().err, panda::pandasm::Error::ErrorType::ERR_NONE) << parser.ShowError().message;
    EXPECT_TRUE(res.HasValue());
    if (!res.HasValue()) {
        return panda::pandasm::Program();
    }
    return std::move(res.Value());
}
}  // namespace

/**
 * @tc.name: remove_console_log_test_001
 * @tc.desc: canonical console.info() is removed
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(RemoveConsoleLogTest, remove_console_log_test_001, TestSize.Level1)
{
    const auto source = R"(
        .language ECMAScript
        .function any startShare(any a0, any a1, any a2) <static> {
            tryldglobalbyname 0x0, "console"
            sta v0
            ldobjbyname 0x1, "info"
            sta v1
            lda.str "startShare"
            sta v2
            lda v1
            callthis1 0x2, v0, v2
            returnundefined
        }
    )";

    auto program = ParseProgram(source);
    const std::string funcName = "startShare:(any,any,any)";
    ASSERT_EQ(CountConsoleLoads(GetFunction(program, funcName)), 1);

    RemoveConsoleLog(program, funcName);

    EXPECT_EQ(CountConsoleLoads(GetFunction(program, funcName)), 0);
    EXPECT_FALSE(HasOpcode(GetFunction(program, funcName), panda::pandasm::Opcode::CALLTHIS1));
}

/**
 * @tc.name: remove_console_log_test_002
 * @tc.desc: startShare with then(() => console.info) plus a later non-sta console load must not abort
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(RemoveConsoleLogTest, remove_console_log_test_002, TestSize.Level1)
{
    // Models: startShare() { console.info("startShare"); promise.then(() => { console.info("xxxx") }); ... }
    // The arrow body is a nested function (thenCallback). The parent may also contain a console load
    // that is not `sta` (acc-optimized / then(console.info) / typeof console). Old code aborted here.
    const auto source = R"(
        .language ECMAScript
        .function any thenCallback(any a0, any a1, any a2) <static> {
            tryldglobalbyname 0x0, "console"
            sta v0
            ldobjbyname 0x1, "info"
            sta v1
            lda.str "xxxx"
            sta v2
            lda v1
            callthis1 0x2, v0, v2
            returnundefined
        }
        .function any startShare(any a0, any a1, any a2) <static> {
            tryldglobalbyname 0x0, "console"
            sta v0
            ldobjbyname 0x1, "info"
            sta v1
            lda.str "startShare"
            sta v2
            lda v1
            callthis1 0x2, v0, v2
            definefunc 0x3, thenCallback, 0x0
            sta v3
            ldundefined
            sta v4
            ldobjbyname 0x4, "then"
            sta v5
            lda v5
            callthis1 0x5, v4, v3
            tryldglobalbyname 0x6, "console"
            ldobjbyname 0x7, "info"
            sta v6
            returnundefined
        }
    )";

    auto program = ParseProgram(source);
    const std::string startShare = "startShare:(any,any,any)";
    const std::string thenCallback = "thenCallback:(any,any,any)";
    ASSERT_EQ(CountConsoleLoads(GetFunction(program, startShare)), 2);
    ASSERT_EQ(CountConsoleLoads(GetFunction(program, thenCallback)), 1);

    RemoveConsoleLog(program, startShare);
    RemoveConsoleLog(program, thenCallback);

    EXPECT_EQ(CountConsoleLoads(GetFunction(program, startShare)), 1);
    EXPECT_TRUE(HasOpcode(GetFunction(program, startShare), panda::pandasm::Opcode::DEFINEFUNC));
    EXPECT_TRUE(HasOpcode(GetFunction(program, startShare), panda::pandasm::Opcode::CALLTHIS1));
    EXPECT_EQ(CountConsoleLoads(GetFunction(program, thenCallback)), 0);
}

/**
 * @tc.name: remove_console_log_test_003
 * @tc.desc: two canonical console.xxx() calls in one function are both removed
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(RemoveConsoleLogTest, remove_console_log_test_003, TestSize.Level1)
{
    const auto source = R"(
        .language ECMAScript
        .function any startShare(any a0, any a1, any a2) <static> {
            tryldglobalbyname 0x0, "console"
            sta v0
            ldobjbyname 0x1, "info"
            sta v1
            lda.str "a"
            sta v2
            lda v1
            callthis1 0x2, v0, v2
            tryldglobalbyname 0x3, "console"
            sta v0
            ldobjbyname 0x4, "info"
            sta v1
            lda.str "b"
            sta v2
            lda v1
            callthis1 0x5, v0, v2
            returnundefined
        }
    )";

    auto program = ParseProgram(source);
    const std::string funcName = "startShare:(any,any,any)";
    ASSERT_EQ(CountConsoleLoads(GetFunction(program, funcName)), 2);

    RemoveConsoleLog(program, funcName);

    EXPECT_EQ(CountConsoleLoads(GetFunction(program, funcName)), 0);
}

/**
 * @tc.name: remove_console_log_test_004
 * @tc.desc: console.log() with callthis0 is removed
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(RemoveConsoleLogTest, remove_console_log_test_004, TestSize.Level1)
{
    const auto source = R"(
        .language ECMAScript
        .function any foo(any a0, any a1, any a2) <static> {
            tryldglobalbyname 0x0, "console"
            sta v0
            ldobjbyname 0x1, "log"
            sta v1
            lda v1
            callthis0 0x2, v0
            returnundefined
        }
    )";

    auto program = ParseProgram(source);
    const std::string funcName = "foo:(any,any,any)";
    ASSERT_EQ(CountConsoleLoads(GetFunction(program, funcName)), 1);

    RemoveConsoleLog(program, funcName);

    EXPECT_EQ(CountConsoleLoads(GetFunction(program, funcName)), 0);
    EXPECT_FALSE(HasOpcode(GetFunction(program, funcName), panda::pandasm::Opcode::CALLTHIS0));
}

/**
 * @tc.name: remove_console_log_test_005
 * @tc.desc: label between tryldglobalbyname console and sta still allows removal
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(RemoveConsoleLogTest, remove_console_log_test_005, TestSize.Level1)
{
    const auto source = R"(
        .language ECMAScript
        .function any foo(any a0, any a1, any a2) <static> {
            tryldglobalbyname 0x0, "console"
        jump_label_0:
            sta v0
            ldobjbyname 0x1, "info"
            sta v1
            lda.str "x"
            sta v2
            lda v1
            callthis1 0x2, v0, v2
            returnundefined
        }
    )";

    auto program = ParseProgram(source);
    const std::string funcName = "foo:(any,any,any)";
    ASSERT_EQ(CountConsoleLoads(GetFunction(program, funcName)), 1);

    RemoveConsoleLog(program, funcName);

    EXPECT_EQ(CountConsoleLoads(GetFunction(program, funcName)), 0);
}

/**
 * @tc.name: remove_console_log_test_006
 * @tc.desc: labels inside the removed console range are kept when jumps outside the range target them
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(RemoveConsoleLogTest, remove_console_log_test_006, TestSize.Level1)
{
    const auto source = R"(
        .language ECMAScript
        .function any getCharData(any a0, any a1, any a2) <static> {
            lda a0
            callruntime.isfalse 0x0
            jnez jump_label_1
            tryldglobalbyname 0x1, "console"
            sta v0
        jump_label_1:
            lda v0
            callruntime.isfalse 0x2
            jnez jump_label_3
            tryldglobalbyname 0x3, "console"
            ldobjbyname 0x4, "warn"
            sta v0
            ldtrue
            stobjbyname 0x5, "_warned", a2
            tryldglobalbyname 0x6, "console"
            sta v0
            ldobjbyname 0x7, "warn"
            sta v1
            lda.str "Missing character"
            sta v2
            mov v5, v0
            mov v6, v2
            lda v1
            callthisrange 0x8, 0x2, v5
        jump_label_3:
            ldundefined
            returnundefined
        }
    )";

    auto program = ParseProgram(source);
    const std::string funcName = "getCharData:(any,any,any)";
    ASSERT_EQ(CountConsoleLoads(GetFunction(program, funcName)), 3);

    RemoveConsoleLog(program, funcName);

    EXPECT_EQ(CountConsoleLoads(GetFunction(program, funcName)), 0);
    EXPECT_FALSE(HasOpcode(GetFunction(program, funcName), panda::pandasm::Opcode::CALLTHISRANGE));
    EXPECT_TRUE(HasJumpTo(GetFunction(program, funcName), "jump_label_1"));
    EXPECT_TRUE(HasLabel(GetFunction(program, funcName), "jump_label_1"));
    EXPECT_TRUE(HasLabel(GetFunction(program, funcName), "jump_label_3"));
}
