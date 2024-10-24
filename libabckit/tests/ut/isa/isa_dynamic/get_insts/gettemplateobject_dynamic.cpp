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

namespace {

auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

auto g_icreateGettemplateobject1Lambda = [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
    auto *worldStr = g_implM->createString(file, "world");
    auto *loadStringWorld = g_dynG->iCreateLoadString(graph, worldStr);
    auto *stringPrint = g_implM->createString(file, "print");
    auto *tryldglobalbyname = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);
    auto *createemptyarray1 = g_dynG->iCreateCreateemptyarray(graph);
    auto *createemptyarray2 = g_dynG->iCreateCreateemptyarray(graph);
    auto *helloStr = g_implM->createString(file, "Hello ");
    auto *loadStringHello = g_dynG->iCreateLoadString(graph, helloStr);
    auto *const0 = g_implG->gCreateConstantU64(graph, 0);
    auto *definefieldbyvalue1 =
        g_dynG->iCreateCallruntimeDefinefieldbyvalue(graph, loadStringHello, const0, createemptyarray1);
    auto *definefieldbyvalue2 =
        g_dynG->iCreateCallruntimeDefinefieldbyvalue(graph, loadStringHello, const0, createemptyarray2);
    auto *str2 = g_implM->createString(file, "!");
    auto *loadString2 = g_dynG->iCreateLoadString(graph, str2);
    auto *const1 = g_implG->gCreateConstantU64(graph, 1);
    auto *definefieldbyvalue3 =
        g_dynG->iCreateCallruntimeDefinefieldbyvalue(graph, loadString2, const1, createemptyarray1);
    auto *definefieldbyvalue4 =
        g_dynG->iCreateCallruntimeDefinefieldbyvalue(graph, loadString2, const1, createemptyarray2);
    auto *createemptyarray3 = g_dynG->iCreateCreateemptyarray(graph);
    auto *definefieldbyvalue5 =
        g_dynG->iCreateCallruntimeDefinefieldbyvalue(graph, createemptyarray1, const0, createemptyarray3);
    auto *definefieldbyvalue6 =
        g_dynG->iCreateCallruntimeDefinefieldbyvalue(graph, createemptyarray2, const1, createemptyarray3);
    auto *gettemplateobject = g_dynG->iCreateGettemplateobject(graph, createemptyarray3);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *deffunc = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
    auto *callargs2 = g_dynG->iCreateCallargs2(graph, deffunc, gettemplateobject, loadStringWorld);
    auto *callarg1 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, callargs2);

    auto *ldundef = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDUNDEFINED);
    g_implG->iInsertBefore(loadStringWorld, ldundef);
    g_implG->iInsertBefore(tryldglobalbyname, ldundef);
    g_implG->iInsertBefore(createemptyarray1, ldundef);
    g_implG->iInsertBefore(createemptyarray2, ldundef);
    g_implG->iInsertBefore(loadStringHello, ldundef);
    g_implG->iInsertBefore(definefieldbyvalue1, ldundef);
    g_implG->iInsertBefore(definefieldbyvalue2, ldundef);
    g_implG->iInsertBefore(loadString2, ldundef);
    g_implG->iInsertBefore(definefieldbyvalue3, ldundef);
    g_implG->iInsertBefore(definefieldbyvalue4, ldundef);
    g_implG->iInsertBefore(createemptyarray3, ldundef);
    g_implG->iInsertBefore(definefieldbyvalue5, ldundef);
    g_implG->iInsertBefore(definefieldbyvalue6, ldundef);
    g_implG->iInsertBefore(gettemplateobject, ldundef);
    g_implG->iInsertBefore(callargs2, ldundef);
    g_implG->iInsertBefore(callarg1, ldundef);
};

}  // namespace

class LibAbcKitCreateDynGettemplateobject : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateGettemplateobject, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitCreateDynGettemplateobject, IcreateGettemplateobject_1)
{
    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/gettemplateobject_dynamic.abc", "gettemplateobject_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/gettemplateobject_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/gettemplateobject_dynamic_modified.abc",
                             "func_main_0", g_icreateGettemplateobject1Lambda);

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/gettemplateobject_dynamic_modified.abc",
                                   "gettemplateobject_dynamic");
    EXPECT_TRUE(helpers::Match(output, "Hello world!\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
