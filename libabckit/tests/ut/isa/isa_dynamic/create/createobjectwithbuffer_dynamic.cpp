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

class LibAbcKitCreateDynCreateobjectwithbuffer : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCreateobjectwithbuffer, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitCreateDynCreateobjectwithbuffer, IcreateCreateobjectwithbuffer_1)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic.abc",
                                   "createobjectwithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto arr = std::vector<AbckitLiteral *>();
            arr.emplace_back(g_implM->createLiteralString(file, "a"));
            arr.emplace_back(g_implM->createLiteralU32(file, 1));
            arr.emplace_back(g_implM->createLiteralString(file, "b"));
            arr.emplace_back(g_implM->createLiteralString(file, "str"));
            auto *litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());

            auto *createobjectwithbuffer = g_dynG->iCreateCreateobjectwithbuffer(graph, litArr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *stringPrint = g_implM->createString(file, "print");
            auto *tryldglobalbyname = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);
            auto *stringA = g_implM->createString(file, "a");
            auto *ldobjbynameA = g_dynG->iCreateLdobjbyname(graph, createobjectwithbuffer, stringA);
            auto *stringB = g_implM->createString(file, "b");
            auto *ldobjbynameB = g_dynG->iCreateLdobjbyname(graph, createobjectwithbuffer, stringB);
            auto *callarg10 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, ldobjbynameA);
            auto *callarg11 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, ldobjbynameB);

            auto *ldundef = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDUNDEFINED);
            g_implG->iInsertBefore(createobjectwithbuffer, ldundef);
            g_implG->iInsertBefore(tryldglobalbyname, ldundef);
            g_implG->iInsertBefore(ldobjbynameA, ldundef);
            g_implG->iInsertBefore(ldobjbynameB, ldundef);
            g_implG->iInsertBefore(callarg10, ldundef);
            g_implG->iInsertBefore(callarg11, ldundef);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic_modified.abc",
                                        "createobjectwithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, "1\nstr\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCreateobjectwithbuffer, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitCreateDynCreateobjectwithbuffer, IcreateCreateobjectwithbuffer_2)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic.abc",
                                   "createobjectwithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto arr = std::vector<AbckitLiteral *>();
            arr.emplace_back(g_implM->createLiteralString(file, "toString"));
            arr.emplace_back(g_implM->createLiteralMethod(file, helpers::FindMethodByName(file, "toString")));
            arr.emplace_back(g_implM->createLiteralMethodAffiliate(file, 0));
            auto *litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());

            auto *createobjectwithbuffer = g_dynG->iCreateCreateobjectwithbuffer(graph, litArr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *stringPrint = g_implM->createString(file, "print");
            auto *tryldglobalbyname = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);
            auto *callarg1 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, createobjectwithbuffer);

            auto *ldundef = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDUNDEFINED);
            g_implG->iInsertBefore(createobjectwithbuffer, ldundef);
            g_implG->iInsertBefore(tryldglobalbyname, ldundef);
            g_implG->iInsertBefore(callarg1, ldundef);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic_modified.abc",
                                        "createobjectwithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, "objA\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCreateobjectwithbuffer, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitCreateDynCreateobjectwithbuffer, IcreateCreateobjectwithbuffer_3)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic.abc",
                                   "createobjectwithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto arr = std::vector<AbckitLiteral *>();
            arr.emplace_back(g_implM->createLiteralString(file, "foo"));
            arr.emplace_back(g_implM->createLiteralMethod(file, helpers::FindMethodByName(file, "foo")));
            arr.emplace_back(g_implM->createLiteralMethodAffiliate(file, 0));
            arr.emplace_back(g_implM->createLiteralString(file, "bar"));
            arr.emplace_back(g_implM->createLiteralMethod(file, helpers::FindMethodByName(file, "bar")));
            arr.emplace_back(g_implM->createLiteralMethodAffiliate(file, 1));
            auto *litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());

            auto *createobjectwithbuffer = g_dynG->iCreateCreateobjectwithbuffer(graph, litArr);
            auto *stringFoo = g_implM->createString(file, "foo");
            auto *stringBar = g_implM->createString(file, "bar");
            auto *ldobjbynameFoo = g_dynG->iCreateLdobjbyname(graph, createobjectwithbuffer, stringFoo);
            auto *ldobjbynameBar = g_dynG->iCreateLdobjbyname(graph, createobjectwithbuffer, stringBar);
            auto *callthisFoo = g_dynG->iCreateCallthis0(graph, ldobjbynameFoo, createobjectwithbuffer);
            auto *callthisBar = g_dynG->iCreateCallthis1(graph, ldobjbynameBar, createobjectwithbuffer,
                                                         g_implG->gFindOrCreateConstantU64(graph, 777));

            auto *ldundef = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDUNDEFINED);
            g_implG->iInsertBefore(createobjectwithbuffer, ldundef);
            g_implG->iInsertBefore(ldobjbynameFoo, ldundef);
            g_implG->iInsertBefore(ldobjbynameBar, ldundef);
            g_implG->iInsertBefore(callthisFoo, ldundef);
            g_implG->iInsertBefore(callthisBar, ldundef);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic_modified.abc",
                                        "createobjectwithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, "foo\nbar 777\n"));
}
}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
