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

class LibAbcKitCppMockArktsTestClass : public ::testing::Test {};

// Test: test-kind=mock, api=Class::RemoveAnnotation, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestClass, Class_RemoveAnnotation)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsClass(f).RemoveAnnotation(abckit::mock::helpers::GetMockArktsAnnotation(f));
        ASSERT_TRUE(CheckMockedApi("ClassRemoveAnnotation"));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationToArktsAnnotation"));
        ASSERT_TRUE(CheckMockedApi("CoreClassToArktsClass"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::AddAnnotation, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestClass, Class_AddAnnotation)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsClass(f).AddAnnotation(
            abckit::mock::helpers::GetMockArktsAnnotationInterface(f));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationToArktsAnnotation"));
        ASSERT_TRUE(CheckMockedApi("ArktsAnnotationToCoreAnnotation"));
        ASSERT_TRUE(CheckMockedApi("ClassAddAnnotation"));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationInterfaceToArktsAnnotationInterface"));
        ASSERT_TRUE(CheckMockedApi("CoreClassToArktsClass"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::cpp_test