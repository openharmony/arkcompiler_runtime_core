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

#include <limits>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "helpers/helpers.h"
#include "libabckit/c/abckit.h"
#include "libabckit/c/isa/isa_static.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "static_core/assembler/assembly-program.h"

namespace libabckit::test {
namespace {

constexpr auto INPUT_ABC = ABCKIT_ABC_DIR "ut/ir_core/remove_log_istrue/remove_log_istrue_static.abc";
constexpr auto OUTPUT_ABC = ABCKIT_ABC_DIR "ut/ir_core/remove_log_istrue/remove_log_istrue_static_modified.abc";
constexpr uint16_t ACC_REG_ID = static_cast<uint16_t>(std::numeric_limits<uint8_t>::max()) - 1U;

bool IsConsoleLogCall(AbckitInst *inst)
{
    if (helpers::g_gStat->iGetOpcode(inst) != ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC) {
        return false;
    }
    AbckitCoreFunction *callee = helpers::g_implG->iGetFunction(inst);
    auto name = helpers::AbckitStringToString(helpers::g_implI->functionGetName(callee));
    return name.find(":std.core.Console;") != std::string::npos;
}

void RemoveConsoleLogCalls(AbckitGraph *graph)
{
    std::vector<AbckitBasicBlock *> blocks;
    helpers::g_implG->gVisitBlocksRpo(graph, &blocks, [](AbckitBasicBlock *bb, void *data) -> bool {
        static_cast<std::vector<AbckitBasicBlock *> *>(data)->push_back(bb);
        return true;
    });

    for (AbckitBasicBlock *bb : blocks) {
        for (AbckitInst *inst = helpers::g_implG->bbGetFirstInst(bb); inst != nullptr;
             inst = helpers::g_implG->iGetNext(inst)) {
            if (IsConsoleLogCall(inst)) {
                helpers::g_implG->iRemove(inst);
                ASSERT_EQ(helpers::g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            }
        }
    }
}

void VerifyEtsIstrueUsesVregNotAcc(ark::pandasm::Program *prog, const std::string &methodNeedle)
{
    for (const auto &[key, func] : prog->functionStaticTable) {
        if (key.find(methodNeedle) == std::string::npos) {
            continue;
        }
        size_t istrueCount = 0;
        for (const auto &ins : func.ins) {
            if (ins.opcode != ark::pandasm::Opcode::ETS_ISTRUE) {
                continue;
            }
            ++istrueCount;
            ASSERT_EQ(ins.RegSize(), 1U) << key;
            const uint16_t reg = ins.GetReg(0U);
            EXPECT_NE(reg, ACC_REG_ID) << "ets.istrue must read vreg, not acc: " << key;
            EXPECT_LT(reg, func.regsNum) << "ets.istrue vreg out of range: " << key;
        }
        ASSERT_GE(istrueCount, 1U) << "expected at least one ets.istrue in " << key;
        return;
    }
    FAIL() << "method not found: " << methodNeedle;
}

void VerifyEtsIstrueUsesVregNotAcc(const std::string &abcPath, const std::string &methodNeedle)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(abcPath.c_str(), &file);
    auto *prog = reinterpret_cast<AbckitFile *>(file)->GetStaticProgram();
    VerifyEtsIstrueUsesVregNotAcc(prog, methodNeedle);
    helpers::g_impl->closeFile(file);
    ASSERT_EQ(helpers::g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace

class LibAbcKitRemoveLogIstrueTest : public ::testing::Test {};

TEST_F(LibAbcKitRemoveLogIstrueTest, RemoveLogPreservesEtsIstrueVreg)
{
    helpers::TransformMethod(INPUT_ABC, OUTPUT_ABC, "checkErrorCode",
                             [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 RemoveConsoleLogCalls(graph);
                             });

    VerifyEtsIstrueUsesVregNotAcc(OUTPUT_ABC, "checkErrorCode");
}

}  // namespace libabckit::test
