/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "libabckit/include/c/metadata_core.h"
#include "libabckit/include/c/ir_core.h"
#include "libabckit/include/c/isa/isa_dynamic.h"
#include "libabckit/include/c/abckit.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)

namespace libabckit::test {

class LibAbcKitDynamicTryCatchTest : public ::testing::Test {};

namespace {
auto *g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto *g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto *g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto *g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

enum class TryCatchScenario {
    DEFAULT_POSITIVE = 0,
};

[[maybe_unused]] std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> CreateBBSchema(TryCatchScenario scenario)
{
    helpers::BBSchema<AbckitIsaApiDynamicOpcode> bb0 {{}, {1}, {}};

    helpers::BBSchema<AbckitIsaApiDynamicOpcode> bb2 {{1}, {}, {}};

    switch (scenario) {
        case TryCatchScenario::DEFAULT_POSITIVE:
            return {bb0,
                    {{0},
                     {2},
                     {
                         {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYOBJECT, {}},
                         {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
                         {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {4}},
                     }},
                    bb2};
        default:
            LIBABCKIT_UNREACHABLE_TEST(DEBUG);
    }
}
}  // namespace

// Test: test-kind=api, api=GraphApiImpl::gInsertTryCatch, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitDynamicTryCatchTest, TryCatchWrapPositive)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/insert_try_catch/insert_try_catch_dynamic_wrapper.abc",
                                   "insert_try_catch_dynamic_wrapper");
    EXPECT_TRUE(helpers::Match(output, "TRY\nTHROW\nOUTER_CATCH\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/insert_try_catch/insert_try_catch_dynamic_wrapper.abc",
        ABCKIT_ABC_DIR "ut/ir_core/insert_try_catch/insert_try_catch_dynamic_wrapper_modified.abc", "bar",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);

            AbckitBasicBlock *tryFirstBB = helpers::BBgetSuccBlocks(startBB)[0];
            AbckitBasicBlock *tryLastBB = g_implG->bbGetTrueBranch(tryFirstBB);

            AbckitBasicBlock *catchFirstBB = g_implG->bbCreateEmpty(graph);
            auto *str = g_dynG->iCreateLoadString(graph, g_implM->createString(file, "INNER_CATCH"));
            g_implG->bbAddInstBack(catchFirstBB, str);
            auto *print = g_dynG->iCreateTryldglobalbyname(graph, g_implM->createString(file, "print"));
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->bbAddInstBack(catchFirstBB, print);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *callArg = g_dynG->iCreateCallarg1(graph, print, str);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->bbAddInstBack(catchFirstBB, callArg);

            g_implG->gInsertTryCatch(tryFirstBB, tryLastBB, catchFirstBB, catchFirstBB);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/ir_core/insert_try_catch/insert_try_catch_dynamic_wrapper_modified.abc",
                                        "insert_try_catch_dynamic_wrapper");
    EXPECT_TRUE(helpers::Match(output, "TRY\nTHROW\nINNER_CATCH\n"));
}

// Test: test-kind=internal, abc-kind=ArkTS1, category=internal
TEST_F(LibAbcKitDynamicTryCatchTest, InsertTryCatchValid)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/insert_try_catch/insert_try_catch_dynamic.abc",
                                             "insert_try_catch_dynamic");
    EXPECT_TRUE(helpers::Match(output, "TRY1\nTRY2\nCATCH\nTRY1\nTRY3\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/insert_try_catch/insert_try_catch_dynamic.abc",
        ABCKIT_ABC_DIR "ut/ir_core/insert_try_catch/insert_try_catch_dynamic_modified.abc", "main",
        []([[maybe_unused]] AbckitFile *file, [[maybe_unused]] AbckitCoreFunction *method,
           [[maybe_unused]] AbckitGraph *graph) {},
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/ir_core/insert_try_catch/insert_try_catch_dynamic_modified.abc", "insert_try_catch_dynamic");
    EXPECT_TRUE(helpers::Match(output, "TRY1\nTRY2\nCATCH\nTRY1\nTRY3\n"));
}

}  // namespace libabckit::test

// NOLINTEND(readability-magic-numbers)
