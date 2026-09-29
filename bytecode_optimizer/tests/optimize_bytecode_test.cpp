/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "assembler/assembly-parser.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "graph_test.h"
#include "mem/pool_manager.h"
#include "optimize_bytecode.h"

using namespace testing::ext;

namespace panda::bytecodeopt {
using namespace compiler;
using namespace panda::pandasm;

class OptimizeBytecodeTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}

    void EmitAndOptimize(const std::string &abc_file_name, panda::pandasm::Program &program) const
    {
        panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps panda_file_to_asm_maps {};
        EXPECT_TRUE(panda::pandasm::AsmEmitter::Emit(abc_file_name, program, nullptr, &panda_file_to_asm_maps, false));
        EXPECT_TRUE(panda::bytecodeopt::OptimizeBytecode(&program, &panda_file_to_asm_maps, abc_file_name, true));
    }
};

/**
 * @tc.name: optimize_bytecode_test_001
 * @tc.desc: Verify the OptimizeBytecode function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(OptimizeBytecodeTest, optimize_bytecode_test_001, TestSize.Level1) {
    const auto source = R"(
        .language ECMAScript
        .array array_unsigned_byte u8 3 { 1 2 3 }
        .record Asm4 { i32 asm1 }
        .function any func_main_0(any a0, any a1, any a2) <static> {
            ldundefined
            stglobalvar 0x0, "j"
            ldundefined
            stglobalvar 0x1, "n"
            lda.str "a b c"
            stglobalvar 0x2, "n"
            ldglobalvar 0x3, "n"
            getpropiterator
            sta v0
        jump_label_1:
            getnextpropname v0
            sta v1
            ldundefined
            eq 0x4, v1
            jnez jump_label_0
            lda v1
            stglobalvar 0x5, "j"
            tryldglobalbyname 0x6, "console"
            sta v1
            ldobjbyname 0x7, "log"
            sta v2
            ldglobalvar 0x9, "n"
            sta v3
            ldglobalvar 0xa, "j"
            ldobjbyvalue 0xb, v3
            sta v3
            lda v2
            callthis1 0xd, v1, v3
            jmp jump_label_1
        jump_label_0:
            ldundefined
            returnundefined
        }
    )";

    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    auto &program = res.Value();
    const std::string fun_name = "func_main_0:(any,any,any)";
    auto it = program.function_table.find(fun_name);
    EXPECT_NE(it, program.function_table.end());
    EmitAndOptimize("codegenTryCatch.abc", program);
}

/**
 * @tc.name: optimize_bytecode_test_002
 * @tc.desc: Verify the OptimizeBytecode function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(OptimizeBytecodeTest, optimize_bytecode_test_002, TestSize.Level1) {
    const auto source = R"(
        .language ECMAScript
        .array array_unsigned_byte u8 3 { 1 2 3 }
        .record Asm4 { i32 asm1 }
        .function any func_main_0(any a0, any a1, any a2) <static> {
            ldundefined
            stglobalvar 0x0, "j"
            ldundefined
            stglobalvar 0x1, "n"
            lda.str "a b c"
            stglobalvar 0x2, "n"
            ldglobalvar 0x3, "n"
            getpropiterator
            sta v0
        jump_label_1:
            getnextpropname v0
            sta v1
            ldundefined
            eq 0x4, v1
            jnez jump_label_0
            lda v1
            stglobalvar 0x5, "j"
            tryldglobalbyname 0x6, "console"
            sta v1
            ldobjbyname 0x7, "log"
            sta v2
            ldglobalvar 0x9, "n"
            sta v3
            ldglobalvar 0xa, "j"
            ldobjbyvalue 0xb, v3
            sta v3
            lda v2
            callthis1 0xd, v1, v3
            jmp jump_label_1
        jump_label_0:
            ldundefined
            returnundefined
        }
    )";
    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    auto &program = res.Value();
    const std::string fun_name = "func_main_0:(any,any,any)";
    auto it = program.function_table.find(fun_name);

    EXPECT_NE(it, program.function_table.end());
    const std::string abc_file_name = "codegenTryCatch.abc";

    panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps panda_file_to_asm_maps {};
    EXPECT_TRUE(panda::pandasm::AsmEmitter::Emit(abc_file_name, program, nullptr, &panda_file_to_asm_maps, false));
}

/**
 * @tc.name: optimize_bytecode_test_003
 * @tc.desc: Verify the OptimizeBytecode function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(OptimizeBytecodeTest, optimize_bytecode_test_003, TestSize.Level1) {
    const auto source = R"(
        .language ECMAScript
        .array array_unsigned_byte u8 3 { 1 2 3 }
        .record Asm4 { i32 asm1 }
        .function any func_main_0(any a0, any a1, any a2) <static> {
            ldundefined
            stglobalvar 0x0, "j"
            ldundefined
            stglobalvar 0x1, "n"
            lda.str "a b c"
            stglobalvar 0x2, "n"
            ldglobalvar 0x3, "n"
            getpropiterator
            sta v0
        jump_label_1:
            getnextpropname v0
            sta v1
            ldundefined
            eq 0x4, v1
            jnez jump_label_0
            lda v1
            stglobalvar 0x5, "j"
            tryldglobalbyname 0x6, "console"
            sta v1
            ldobjbyname 0x7, "log"
            sta v2
            ldglobalvar 0x9, "n"
            sta v3
            ldglobalvar 0xa, "j"
            ldobjbyvalue 0xb, v3
            sta v3
            lda v2
            callthis1 0xd, v1, v3
            jmp jump_label_1
        jump_label_0:
            ldundefined
            returnundefined
        }
    )";

    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    auto &program = res.Value();
    const std::string fun_name = "func_main_0:(any,any,any)";
    auto it = program.function_table.find(fun_name);

    Function::CatchBlock cat;
    cat.try_end_label = "random";  // random: It's a random string
    it->second.catch_blocks.push_back(cat);
    EXPECT_NE(it, program.function_table.end());

    const std::string abc_file_name = "codegenTryCatch.abc";

    panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps panda_file_to_asm_maps {};
    EXPECT_TRUE(panda::pandasm::AsmEmitter::Emit(abc_file_name, program, nullptr, &panda_file_to_asm_maps, false));
    EXPECT_TRUE(panda::bytecodeopt::OptimizeBytecode(&program, &panda_file_to_asm_maps, abc_file_name, true));
}

HWTEST_F(OptimizeBytecodeTest, EliminatesRepeatedDynamicStrictEqInPipeline, TestSize.Level1)
{
    const auto source = R"(
        .language ECMAScript
        .function any func_main_0(any a0, any a1) <static> {
            lda a0
            stricteq 0x0, a1
            sta v0
            lda a0
            stricteq 0x1, a1
            sta v1
            lda v1
            istrue
            sta v2
            lda v0
            typeof 0x2
            sta v3
            lda v2
            add2 0x3, v3
            return
        }
    )";

    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res.HasValue());
    auto &program = res.Value();
    const std::string funName = "func_main_0:(any,any)";
    auto it = program.function_table.find(funName);
    ASSERT_NE(it, program.function_table.end());

    EmitAndOptimize("dynamicCsePositive.abc", program);

    const auto &instructions = it->second.ins;
    auto strictEqCount = std::count_if(instructions.begin(), instructions.end(), [](const auto &inst) {
        return inst->GetOpcode() == pandasm::Opcode::STRICTEQ;
    });
    EXPECT_EQ(strictEqCount, 1U);
}

HWTEST_F(OptimizeBytecodeTest, KeepsRepeatedDynamicStrictEqWhenProducerIsNotLiveAcrossCurrent, TestSize.Level1)
{
    const auto source = R"(
        .language ECMAScript
        .function any func_main_0(any a0, any a1) <static> {
            lda a0
            stricteq 0x0, a1
            sta v0
            lda v0
            typeof 0x1
            sta v1
            lda a0
            stricteq 0x2, a1
            sta v2
            lda v2
            add2 0x3, v1
            return
        }
    )";

    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res.HasValue());
    auto &program = res.Value();
    const std::string funName = "func_main_0:(any,any)";
    auto it = program.function_table.find(funName);
    ASSERT_NE(it, program.function_table.end());

    EmitAndOptimize("dynamicCseNegative.abc", program);

    const auto &instructions = it->second.ins;
    auto strictEqCount = std::count_if(instructions.begin(), instructions.end(), [](const auto &inst) {
        return inst->GetOpcode() == pandasm::Opcode::STRICTEQ;
    });
    EXPECT_EQ(strictEqCount, 2U);
}

HWTEST_F(OptimizeBytecodeTest, EliminatesRepeatedSuperCallStateChecksInPipeline, TestSize.Level1)
{
    const auto source = R"(
        .language ECMAScript
        .function any func_main_0(any a0) <static> {
            lda a0
            throw.ifsupernotcorrectcall 0x0
            sta v0
            lda a0
            throw.ifsupernotcorrectcall 0x0
            return
        }
    )";

    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res.HasValue());
    auto &program = res.Value();
    const std::string funName = "func_main_0:(any)";
    auto it = program.function_table.find(funName);
    ASSERT_NE(it, program.function_table.end());

    const auto CountOpcode = [](const auto &instructions, pandasm::Opcode opcode) {
        return std::count_if(instructions.begin(), instructions.end(),
                             [opcode](const auto &inst) { return inst->GetOpcode() == opcode; });
    };
    auto ldaCountBefore = CountOpcode(it->second.ins, pandasm::Opcode::LDA);
    auto staCountBefore = CountOpcode(it->second.ins, pandasm::Opcode::STA);

    EmitAndOptimize("redundantSuperCheck.abc", program);

    const auto &instructions = it->second.ins;
    EXPECT_EQ(CountOpcode(instructions, pandasm::Opcode::THROW_IFSUPERNOTCORRECTCALL), 1U);
    EXPECT_LE(CountOpcode(instructions, pandasm::Opcode::LDA), ldaCountBefore);
    EXPECT_LE(CountOpcode(instructions, pandasm::Opcode::STA), staCountBefore);
}

HWTEST_F(OptimizeBytecodeTest, KeepsSuperChecksAcrossThisInitializationInPipeline, TestSize.Level1)
{
    const auto source = R"(
        .language ECMAScript
        .function any func_main_0(any a0, any a1) <static> {
            lda a0
            sta v0
            lda v0
            throw.ifsupernotcorrectcall 0x1
            lda a1
            sta v0
            lda v0
            throw.ifsupernotcorrectcall 0x0
            return
        }
    )";

    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res.HasValue());
    auto &program = res.Value();
    const std::string funName = "func_main_0:(any,any)";
    auto it = program.function_table.find(funName);
    ASSERT_NE(it, program.function_table.end());

    EmitAndOptimize("superCheckStateTransition.abc", program);

    const auto &instructions = it->second.ins;
    auto checkCount = std::count_if(instructions.begin(), instructions.end(), [](const auto &inst) {
        return inst->GetOpcode() == pandasm::Opcode::THROW_IFSUPERNOTCORRECTCALL;
    });
    EXPECT_EQ(checkCount, 2U);
}
}  // namespace panda::bytecodeopt
