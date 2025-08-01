/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "libabckit/include/c/isa/isa_static.h"
#include "libabckit/include/c/abckit.h"

#include "helpers/helpers.h"
#include "ut/isa/isa_dynamic/arithmetic/helpers_arithmetic.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitBinInstShiftsImmTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateAShrI, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBinInstShiftsImmTest, CreateAShrIValid)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_shifts_imm_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_shifts_imm_static_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::arithmetic::TransformIrBinInstrWithImmValid(graph, g_statG->iCreateAShrI);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                helpers::arithmetic::CreateBBSchemaForBinaryWithImm(ABCKIT_ISA_API_STATIC_OPCODE_ASHRI));
            helpers::VerifyGraph(graph, bbSchemas);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateShrI, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBinInstShiftsImmTest, CreateShrIValid)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_shifts_imm_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_shifts_imm_static_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::arithmetic::TransformIrBinInstrWithImmValid(graph, g_statG->iCreateShrI);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                helpers::arithmetic::CreateBBSchemaForBinaryWithImm(ABCKIT_ISA_API_STATIC_OPCODE_SHRI));
            helpers::VerifyGraph(graph, bbSchemas);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateShlI, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBinInstShiftsImmTest, CreateShlIValid)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_shifts_imm_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_shifts_imm_static_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::arithmetic::TransformIrBinInstrWithImmValid(graph, g_statG->iCreateShlI);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                helpers::arithmetic::CreateBBSchemaForBinaryWithImm(ABCKIT_ISA_API_STATIC_OPCODE_SHLI));
            helpers::VerifyGraph(graph, bbSchemas);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
