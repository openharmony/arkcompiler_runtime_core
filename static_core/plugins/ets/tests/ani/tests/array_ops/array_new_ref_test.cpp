/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#include "ani_gtest.h"
#include <iostream>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ArrayNewRefTest : public AniTest {
public:
    static constexpr const ani_size LENGTH = 5;
    static constexpr const ani_size ZERO = 0;
};

TEST_F(ArrayNewRefTest, NewRefErrorTests)
{
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("Lstd/core/String;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_array_ref array = nullptr;
    // Test null result pointer
    ASSERT_EQ(env_->Array_New_Ref(cls, LENGTH, nullptr, nullptr), ANI_INVALID_ARGS);

    // Test null class
    ASSERT_EQ(env_->Array_New_Ref(nullptr, LENGTH, nullptr, &array), ANI_INVALID_ARGS);

    if (sizeof(ani_size) > sizeof(uint32_t)) {
        ani_size maxLength = std::numeric_limits<uint32_t>::max() + ani_size(1);
        ASSERT_EQ(env_->Array_New_Ref(cls, maxLength, nullptr, &array), ANI_INVALID_ARGS);
    }
}

TEST_F(ArrayNewRefTest, NewObjectArrayTest)
{
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("Lstd/core/String;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    // Test zero length
    ani_array_ref zeroLengthArray = nullptr;
    ASSERT_EQ(env_->Array_New_Ref(cls, ZERO, nullptr, &zeroLengthArray), ANI_OK);
    ASSERT_NE(zeroLengthArray, nullptr);
    ani_size zeroLengthSize = 0;
    ASSERT_EQ(env_->Array_GetLength(zeroLengthArray, &zeroLengthSize), ANI_OK);
    ASSERT_EQ(zeroLengthSize, ZERO);

    ani_array_ref array = nullptr;

    // Test creating array with null initial element
    ASSERT_EQ(env_->Array_New_Ref(cls, LENGTH, nullptr, &array), ANI_OK);
    ASSERT_NE(array, nullptr);
    ani_size size = 0;
    ASSERT_EQ(env_->Array_GetLength(array, &size), ANI_OK);
    ASSERT_EQ(size, LENGTH);

    // Test creating array with undefined initial element
    ani_ref undefinedRef = nullptr;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ASSERT_EQ(env_->Array_New_Ref(cls, LENGTH, undefinedRef, &array), ANI_OK);
    ASSERT_NE(array, nullptr);
    ASSERT_EQ(env_->Array_GetLength(array, &size), ANI_OK);
    ASSERT_EQ(size, LENGTH);

    // Test creating array with null initial element
    ani_ref nullRef = nullptr;
    ASSERT_EQ(env_->GetNull(&nullRef), ANI_OK);
    ASSERT_EQ(env_->Array_New_Ref(cls, LENGTH, nullRef, &array), ANI_OK);
    ASSERT_NE(array, nullptr);
    ASSERT_EQ(env_->Array_GetLength(array, &size), ANI_OK);
    ASSERT_EQ(size, LENGTH);

    // Test creating array with initial element
    ani_string str = nullptr;
    const char *utf8String = "test";
    const ani_size stringLength = strlen(utf8String);
    ASSERT_EQ(env_->String_NewUTF8(utf8String, stringLength, &str), ANI_OK);
    ASSERT_NE(str, nullptr);
    ani_array_ref array2 = nullptr;
    ASSERT_EQ(env_->Array_New_Ref(cls, stringLength, str, &array2), ANI_OK);
    ASSERT_NE(array2, nullptr);

    // Verify initial element was set for all elements
    for (ani_size i = 0; i < stringLength; i++) {
        ani_ref element = nullptr;
        ASSERT_EQ(env_->Array_Get_Ref(array2, i, &element), ANI_OK);
        ani_size resultSize = 0;
        const ani_size utfBufferSize = 10;
        char utfBuffer[utfBufferSize] = {0};
        ASSERT_EQ(env_->String_GetUTF8SubString(reinterpret_cast<ani_string>(element), ZERO, stringLength, utfBuffer,
                                                sizeof(utfBuffer), &resultSize),
                  ANI_OK);
        ASSERT_STREQ(utfBuffer, utf8String);
    }
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
