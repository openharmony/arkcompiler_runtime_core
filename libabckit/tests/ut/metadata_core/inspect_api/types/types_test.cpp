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

class LibAbcKitModifyApiTypesTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::typeGetTypeId, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitModifyApiTypesTest, TypeGetTypeIdDynamic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/types/types_dynamic.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::InspectMethod(file, "foo", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitInst *inst = g_implG->bbGetFirstInst(startBB);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitType *type = g_implG->iGetType(inst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitTypeId gotTypeId = g_implI->typeGetTypeId(type);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(gotTypeId, ABCKIT_TYPE_ID_ANY);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=InspectApiImpl::typeGetTypeId, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiTypesTest, TypeGetTypeIdStatic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/types/types_static.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::InspectMethod(file, "foo", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitInst *inst = g_implG->iGetNext(g_implG->bbGetFirstInst(startBB));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitType *type = g_implG->iGetType(inst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitTypeId gotTypeId = g_implI->typeGetTypeId(type);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(gotTypeId, ABCKIT_TYPE_ID_I32);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=InspectApiImpl::typeGetReferenceClass, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiTypesTest, TypeGetReferenceClassStatic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/types/types_static.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::InspectMethod(file, "foo", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitInst *inst = g_implG->bbGetFirstInst(startBB);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        inst = g_implG->iGetNext(g_implG->iGetNext(inst));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitType *type = g_implG->iGetType(inst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitTypeId gotTypeId = g_implI->typeGetTypeId(type);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(gotTypeId, ABCKIT_TYPE_ID_REFERENCE);
        AbckitCoreClass *klass = g_implI->typeGetReferenceClass(type);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(klass, nullptr);  // Fix to get valid AbckitCoreClass *, not nullptr
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=InspectApiImpl::typeGetReferenceClass, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitModifyApiTypesTest, TypeGetReferenceClassDynamic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/types/types_dynamic.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::InspectMethod(file, "foo", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitInst *inst = g_implG->bbGetFirstInst(startBB);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitType *type = g_implG->iGetType(inst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitTypeId gotTypeId = g_implI->typeGetTypeId(type);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(gotTypeId, ABCKIT_TYPE_ID_REFERENCE);
        AbckitCoreClass *klass = g_implI->typeGetReferenceClass(type);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(klass, nullptr);
    });
    g_impl->closeFile(file);
}

}  // namespace libabckit::test
