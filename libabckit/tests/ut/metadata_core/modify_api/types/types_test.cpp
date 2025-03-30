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

#include <gtest/gtest.h>
#include <cstring>

#include "libabckit/include/c/abckit.h"
#include "libabckit/include/c/statuses.h"
#include "helpers/helpers.h"
#include "libabckit/include/c/ir_core.h"
#include "libabckit/include/c/metadata_core.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_staG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitModifyApiTypesTest : public ::testing::Test {};

static void InspectMethodValid(AbckitFile *file)
{
    std::vector<AbckitTypeId> vecOfTypes = {
        ABCKIT_TYPE_ID_VOID, ABCKIT_TYPE_ID_U1,  ABCKIT_TYPE_ID_I8,  ABCKIT_TYPE_ID_U8,          ABCKIT_TYPE_ID_I16,
        ABCKIT_TYPE_ID_U16,  ABCKIT_TYPE_ID_I32, ABCKIT_TYPE_ID_U32, ABCKIT_TYPE_ID_F32,         ABCKIT_TYPE_ID_F64,
        ABCKIT_TYPE_ID_I64,  ABCKIT_TYPE_ID_U64, ABCKIT_TYPE_ID_ANY, ABCKIT_TYPE_ID_LITERALARRAY};
    for (AbckitTypeId typeId : vecOfTypes) {
        AbckitType *type = g_implM->createType(file, typeId);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitTypeId gotTypeId = g_implI->typeGetTypeId(type);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(gotTypeId, typeId);
    }
}

// Test: test-kind=api, api=ModifyApiImpl::createType, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitModifyApiTypesTest, CreateTypesDynamic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/types/types_dynamic.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::InspectMethod(file, "foo",
                           [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *) { InspectMethodValid(file); });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ModifyApiImpl::createType, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiTypesTest, CreateTypeStatic)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/types/types_static.abc", "foo",
                           [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *) { InspectMethodValid(file); });
}

// Test: test-kind=api, api=ModifyApiImpl::createReferenceType, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitModifyApiTypesTest, CreateReferenceTypeDynamic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/types/types_dynamic.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::InspectMethod(file, "foo2", [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *) {
        auto *meth = helpers::FindMethodByName(file, "foo");
        ASSERT_NE(meth, nullptr);
        auto *cls = g_implI->functionGetParentClass(meth);
        ASSERT_NE(cls, nullptr);
        auto *type = g_implM->createReferenceType(file, cls);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(type, nullptr);
        ASSERT_EQ(g_implI->typeGetTypeId(type), ABCKIT_TYPE_ID_REFERENCE);
        ASSERT_EQ(g_implI->typeGetReferenceClass(type), cls);
    });
    g_impl->closeFile(file);
}

}  // namespace libabckit::test
