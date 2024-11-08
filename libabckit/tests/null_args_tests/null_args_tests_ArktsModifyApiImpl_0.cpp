/*
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

// Autogenerated file by gen_null_arg_tests.rb script -- DO NOT EDIT!

#include "libabckit/include/c/abckit.h"
#include "libabckit/include/c/metadata_core.h"
#include "libabckit/include/c/extensions/arkts/metadata_arkts.h"
#include "libabckit/include/c/extensions/js/metadata_js.h"
#include "libabckit/include/c/ir_core.h"
#include "libabckit/include/c/isa/isa_dynamic.h"
#include "libabckit/src/include_v2/c/isa/isa_static.h"

#include "helpers/helpers.h"
#include "helpers/helpers_nullptr.h"

#include <gtest/gtest.h>

namespace libabckit::test {

static auto g_arktsModifyApiImp = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitNullptrTestsArktsModifyApiImpl0 : public ::testing::Test {};

// Test: test-kind=api, api=ArktsModifyApiImpl::annotationAddAnnotationElement,
// abc-kind=NoABC, category=negative-nullptr
TEST_F(LibAbcKitNullptrTestsArktsModifyApiImpl0, annotationAddAnnotationElement)
{
    helpers_nullptr::TestNullptr(g_arktsModifyApiImp->annotationAddAnnotationElement);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::annotationInterfaceAddField,
// abc-kind=NoABC, category=negative-nullptr
TEST_F(LibAbcKitNullptrTestsArktsModifyApiImpl0, annotationInterfaceAddField)
{
    helpers_nullptr::TestNullptr(g_arktsModifyApiImp->annotationInterfaceAddField);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::annotationInterfaceRemoveField,
// abc-kind=NoABC, category=negative-nullptr
TEST_F(LibAbcKitNullptrTestsArktsModifyApiImpl0, annotationInterfaceRemoveField)
{
    helpers_nullptr::TestNullptr(g_arktsModifyApiImp->annotationInterfaceRemoveField);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::annotationRemoveAnnotationElement,
// abc-kind=NoABC, category=negative-nullptr
TEST_F(LibAbcKitNullptrTestsArktsModifyApiImpl0, annotationRemoveAnnotationElement)
{
    helpers_nullptr::TestNullptr(g_arktsModifyApiImp->annotationRemoveAnnotationElement);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::classAddAnnotation,
// abc-kind=NoABC, category=negative-nullptr
TEST_F(LibAbcKitNullptrTestsArktsModifyApiImpl0, classAddAnnotation)
{
    helpers_nullptr::TestNullptr(g_arktsModifyApiImp->classAddAnnotation);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::classRemoveAnnotation,
// abc-kind=NoABC, category=negative-nullptr
TEST_F(LibAbcKitNullptrTestsArktsModifyApiImpl0, classRemoveAnnotation)
{
    helpers_nullptr::TestNullptr(g_arktsModifyApiImp->classRemoveAnnotation);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::fileAddExternalModuleArktsV1,
// abc-kind=NoABC, category=negative-nullptr
TEST_F(LibAbcKitNullptrTestsArktsModifyApiImpl0, fileAddExternalModuleArktsV1)
{
    helpers_nullptr::TestNullptr(g_arktsModifyApiImp->fileAddExternalModuleArktsV1);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::functionAddAnnotation,
// abc-kind=NoABC, category=negative-nullptr
TEST_F(LibAbcKitNullptrTestsArktsModifyApiImpl0, functionAddAnnotation)
{
    helpers_nullptr::TestNullptr(g_arktsModifyApiImp->functionAddAnnotation);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::functionRemoveAnnotation,
// abc-kind=NoABC, category=negative-nullptr
TEST_F(LibAbcKitNullptrTestsArktsModifyApiImpl0, functionRemoveAnnotation)
{
    helpers_nullptr::TestNullptr(g_arktsModifyApiImp->functionRemoveAnnotation);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddAnnotationInterface,
// abc-kind=NoABC, category=negative-nullptr
TEST_F(LibAbcKitNullptrTestsArktsModifyApiImpl0, moduleAddAnnotationInterface)
{
    helpers_nullptr::TestNullptr(g_arktsModifyApiImp->moduleAddAnnotationInterface);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddExportFromArktsV1ToArktsV1,
// abc-kind=NoABC, category=negative-nullptr
TEST_F(LibAbcKitNullptrTestsArktsModifyApiImpl0, moduleAddExportFromArktsV1ToArktsV1)
{
    helpers_nullptr::TestNullptr(g_arktsModifyApiImp->moduleAddExportFromArktsV1ToArktsV1);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddImportFromArktsV1ToArktsV1,
// abc-kind=NoABC, category=negative-nullptr
TEST_F(LibAbcKitNullptrTestsArktsModifyApiImpl0, moduleAddImportFromArktsV1ToArktsV1)
{
    helpers_nullptr::TestNullptr(g_arktsModifyApiImp->moduleAddImportFromArktsV1ToArktsV1);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleRemoveExport,
// abc-kind=NoABC, category=negative-nullptr
TEST_F(LibAbcKitNullptrTestsArktsModifyApiImpl0, moduleRemoveExport)
{
    helpers_nullptr::TestNullptr(g_arktsModifyApiImp->moduleRemoveExport);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleRemoveImport,
// abc-kind=NoABC, category=negative-nullptr
TEST_F(LibAbcKitNullptrTestsArktsModifyApiImpl0, moduleRemoveImport)
{
    helpers_nullptr::TestNullptr(g_arktsModifyApiImp->moduleRemoveImport);
}

}  // namespace libabckit::test
