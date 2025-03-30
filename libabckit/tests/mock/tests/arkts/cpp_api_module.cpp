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

#include "include/cpp/abckit_cpp.h"
#include "tests/mock/check_mock.h"
#include "src/mock/mock_values.h"
#include "tests/mock/cpp_helpers_mock.h"
#include <gtest/gtest.h>

namespace libabckit::cpp_test {

class LibAbcKitCppMockArktsTestModule : public ::testing::Test {};

// Test: test-kind=mock, api=Module::AddImportFromArktsV1ToArktsV1, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestModule, Module_AddImportFromArktsV1ToArktsV1)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsModule(f).AddImportFromArktsV1ToArktsV1(
            abckit::mock::helpers::GetMockArktsModule(f), DEFAULT_CONST_CHAR, DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("CoreImportDescriptorToArktsImportDescriptor"));
        ASSERT_TRUE(CheckMockedApi("ArktsImportDescriptorToCoreImportDescriptor"));
        ASSERT_TRUE(CheckMockedApi("ModuleAddImportFromArktsV1ToArktsV1"));
        ASSERT_TRUE(CheckMockedApi("CoreModuleToArktsModule"));
        ASSERT_TRUE(CheckMockedApi("CoreModuleToArktsModule"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::RemoveImport, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestModule, Module_RemoveImport)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto id = abckit::mock::helpers::GetMockArktsImportDescriptor(f);
        abckit::mock::helpers::GetMockArktsModule(f).RemoveImport(id);
        ASSERT_TRUE(CheckMockedApi("ModuleRemoveImport"));
        ASSERT_TRUE(CheckMockedApi("CoreModuleToArktsModule"));
        ASSERT_TRUE(CheckMockedApi("CoreImportDescriptorToArktsImportDescriptor"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::AddExportFromArktsV1ToArktsV1, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestModule, Module_AddExportFromArktsV1ToArktsV1)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto m = abckit::mock::helpers::GetMockArktsModule(f);
        abckit::mock::helpers::GetMockArktsModule(f).AddExportFromArktsV1ToArktsV1(m, DEFAULT_CONST_CHAR,
                                                                                   DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("CoreExportDescriptorToArktsExportDescriptor"));
        ASSERT_TRUE(CheckMockedApi("ArktsExportDescriptorToCoreExportDescriptor"));
        ASSERT_TRUE(CheckMockedApi("ModuleAddExportFromArktsV1ToArktsV1"));
        ASSERT_TRUE(CheckMockedApi("CoreModuleToArktsModule"));
        ASSERT_TRUE(CheckMockedApi("CoreModuleToArktsModule"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::RemoveExport, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestModule, Module_RemoveExport)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto ed = abckit::mock::helpers::GetMockArktsExportDescriptor(f);
        abckit::mock::helpers::GetMockArktsModule(f).RemoveExport(ed);
        ASSERT_TRUE(CheckMockedApi("ModuleRemoveExport"));
        ASSERT_TRUE(CheckMockedApi("CoreModuleToArktsModule"));
        ASSERT_TRUE(CheckMockedApi("CoreExportDescriptorToArktsExportDescriptor"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::AddAnnotationInterface, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestModule, Module_AddAnnotationInterface)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsModule(f).AddAnnotationInterface(DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationInterfaceToArktsAnnotationInterface"));
        ASSERT_TRUE(CheckMockedApi("ArktsAnnotationInterfaceToCoreAnnotationInterface"));
        ASSERT_TRUE(CheckMockedApi("ModuleAddAnnotationInterface"));
        ASSERT_TRUE(CheckMockedApi("CoreModuleToArktsModule"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::cpp_test