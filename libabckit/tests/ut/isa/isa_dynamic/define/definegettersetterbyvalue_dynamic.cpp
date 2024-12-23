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

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitCreateDynDefineGetterSetter : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDefinegettersetterbyvalue, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitCreateDynDefineGetterSetter, IcreateDefinegettersetterbyvalue_1)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic.abc",
                                   "definegettersetterbyvalue_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *getterFunc = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            auto *createObj = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
            auto *ldundefined = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDUNDEFINED);
            auto *ldfalse = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDFALSE);
            auto *getterStr = g_implM->createString(file, "getter");
            auto *loadString = g_dynG->iCreateLoadString(graph, getterStr);
            auto *definegetter = g_dynG->iCreateDefinegettersetterbyvalue(graph, ldfalse, createObj, loadString,
                                                                          getterFunc, ldundefined);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *ldundefinedLast = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDUNDEFINED);
            g_implG->iInsertBefore(loadString, ldundefinedLast);
            g_implG->iInsertBefore(definegetter, ldundefinedLast);

            auto *stringPrint = g_implM->createString(file, "print");
            auto *ldglobal = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);
            auto *ldobjbyname = g_dynG->iCreateLdobjbyname(graph, createObj, getterStr);
            auto *callarg = g_dynG->iCreateCallarg1(graph, ldglobal, ldobjbyname);
            g_implG->iInsertBefore(ldglobal, ldundefinedLast);
            g_implG->iInsertBefore(ldobjbyname, ldundefinedLast);
            g_implG->iInsertBefore(callarg, ldundefinedLast);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic_modified.abc",
                                        "definegettersetterbyvalue_dynamic");
    EXPECT_TRUE(helpers::Match(output, "getter\n123\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDefinegettersetterbyvalue, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitCreateDynDefineGetterSetter, IcreateDefinegettersetterbyvalue_2)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic.abc",
                                   "definegettersetterbyvalue_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *setterFunc = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            auto *createObj = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
            auto *ldundefined = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDUNDEFINED);
            auto *ldfalse = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDFALSE);
            auto *setterStr = g_implM->createString(file, "setter");
            auto *loadString = g_dynG->iCreateLoadString(graph, setterStr);
            auto *definesetter = g_dynG->iCreateDefinegettersetterbyvalue(graph, ldfalse, createObj, loadString,
                                                                          ldundefined, setterFunc);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *ldundefinedLast = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDUNDEFINED);
            g_implG->iInsertBefore(loadString, ldundefinedLast);
            g_implG->iInsertBefore(definesetter, ldundefinedLast);

            auto *const1 = g_implG->gFindOrCreateConstantU64(graph, 5);
            auto *stobj = g_dynG->iCreateStobjbyname(graph, const1, setterStr, createObj);
            g_implG->iInsertBefore(stobj, ldundefinedLast);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic_modified.abc",
                                        "definegettersetterbyvalue_dynamic");
    EXPECT_TRUE(helpers::Match(output, "setter\n5\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDefinegettersetterbyvalue, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitCreateDynDefineGetterSetter, IcreateDefinegettersetterbyvalue_3)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic.abc",
                                   "definegettersetterbyvalue_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *getterFunc = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            auto *createObj = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
            auto *ldfalse = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDFALSE);
            auto *getterStr = g_implM->createString(file, "getter");
            auto *loadString = g_dynG->iCreateLoadString(graph, getterStr);
            auto *definesetter =
                g_dynG->iCreateDefinegettersetterbyvalue(graph, ldfalse, createObj, loadString, getterFunc, getterFunc);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *ldundefinedLast = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDUNDEFINED);
            g_implG->iInsertBefore(loadString, ldundefinedLast);
            g_implG->iInsertBefore(definesetter, ldundefinedLast);

            auto *const1 = g_implG->gFindOrCreateConstantU64(graph, 5);
            auto *stobj = g_dynG->iCreateStobjbyname(graph, const1, getterStr, createObj);
            g_implG->iInsertBefore(stobj, ldundefinedLast);

            auto *stringPrint = g_implM->createString(file, "print");
            auto *ldglobal = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);
            auto *ldobjbyname = g_dynG->iCreateLdobjbyname(graph, createObj, getterStr);
            auto *callarg = g_dynG->iCreateCallarg1(graph, ldglobal, ldobjbyname);
            g_implG->iInsertBefore(ldglobal, ldundefinedLast);
            g_implG->iInsertBefore(ldobjbyname, ldundefinedLast);
            g_implG->iInsertBefore(callarg, ldundefinedLast);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic_modified.abc",
                                        "definegettersetterbyvalue_dynamic");
    EXPECT_TRUE(helpers::Match(output, "getter\ngetter\n123\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
