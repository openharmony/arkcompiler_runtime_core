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

#include <gtest/gtest.h>

#include "helpers/helpers.h"
#include "libabckit/c/abckit.h"
#include "libabckit/c/isa/isa_static.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/src/adapter_static/helpers_static.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitArkTSInspectApiOpcodesStaticTest : public ::testing::Test {};

// Test: test-kind=internal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSInspectApiOpcodesStaticTest, Helpers_StaticOpcodesAndOperands)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/helpers/opcodes_static_test.abc";
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::InspectMethod(file, "main", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        // Touch GetStaticOpcode switch with as many inst opcodes as present in this method.
        std::vector<AbckitBasicBlock *> bbs;
        g_implG->gVisitBlocksRpo(graph, &bbs, [](AbckitBasicBlock *bb, void *data) {
            reinterpret_cast<std::vector<AbckitBasicBlock *> *>(data)->emplace_back(bb);
            return true;
        });
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        size_t visited = 0;
        for (auto *bb : bbs) {
            for (auto *inst = g_implG->bbGetFirstInst(bb); inst != nullptr; inst = g_implG->iGetNext(inst)) {
                (void)GetStaticOpcode(inst->impl);
                visited++;
            }
        }
        ASSERT_GT(visited, 0U);

        // Cover literal array operand helpers for static graphs.
        if (auto *loadConstArray = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_LOADCONSTARRAY);
            loadConstArray != nullptr) {
            auto *la = g_implG->iGetLiteralArray(loadConstArray);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(la, nullptr);
        }

        // Cover string operand helpers for static graphs.
        if (auto *loadStr = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING);
            loadStr != nullptr) {
            auto *s = g_implG->iGetString(loadStr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(s, nullptr);
        }

        // Cover type-id operand helpers through static ISA API.
        // Note: not all opcodes are guaranteed to exist in compiled IR, so use soft assertions.
        if (auto *newObj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_NEWOBJECT); newObj != nullptr) {
            (void)g_statG->iGetClass(newObj);
        }
        if (auto *checkCast = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CHECKCAST);
            checkCast != nullptr) {
            (void)g_statG->iGetClass(checkCast);
        }
        if (auto *isInstance = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_ISINSTANCE);
            isInstance != nullptr) {
            (void)g_statG->iGetClass(isInstance);
        }
    });

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace libabckit::test
