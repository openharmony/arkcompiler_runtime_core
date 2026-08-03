/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "fixedarray_gtest_ops.h"
#include <cstdint>
#include <iostream>
#include <limits>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class FixedArrayNewTest : public AniGTestFixedArrayOps {
public:
    static constexpr const ani_size ZERO = 0;

    static constexpr ani_size MINI_LENGTH = 10;
    static constexpr ani_size MID_LENGTH = 50;
    static constexpr ani_size BIG_LENGTH = 200;
};

TEST_F(FixedArrayNewTest, NewErrorTests)
{
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("std.core.String", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_fixedarray array = nullptr;
    // Test null result pointer
    ASSERT_EQ(env_->FixedArray_New(cls, LENGTH_5, nullptr, nullptr), ANI_INVALID_ARGS);

    // Test null class
    ASSERT_EQ(env_->FixedArray_New(nullptr, LENGTH_5, nullptr, &array), ANI_INVALID_ARGS);

    if (sizeof(ani_size) > sizeof(uint32_t)) {
        ani_size maxLength = std::numeric_limits<uint32_t>::max() + ani_size(1);
        ASSERT_EQ(env_->FixedArray_New(cls, maxLength, nullptr, &array), ANI_INVALID_ARGS);
    }
}

TEST_F(FixedArrayNewTest, NewObjectArrayTest)
{
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("std.core.String", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    // Test zero length
    ani_fixedarray zeroLengthArray = nullptr;
    ASSERT_EQ(env_->FixedArray_New(cls, ZERO, nullptr, &zeroLengthArray), ANI_OK);
    ASSERT_NE(zeroLengthArray, nullptr);
    ani_size zeroLengthSize = 0;
    ASSERT_EQ(env_->FixedArray_GetLength(zeroLengthArray, &zeroLengthSize), ANI_OK);
    ASSERT_EQ(zeroLengthSize, ZERO);

    ani_fixedarray array = nullptr;
    ani_size size = 0;

    // Test creating array with undefined initial element
    ani_ref undefinedRef = nullptr;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New(cls, LENGTH_5, undefinedRef, &array), ANI_OK);
    ASSERT_NE(array, nullptr);
    ASSERT_EQ(env_->FixedArray_GetLength(array, &size), ANI_OK);
    ASSERT_EQ(size, LENGTH_5);

    // Test creating array with null initial element
    ani_ref nullRef = nullptr;
    ASSERT_EQ(env_->GetNull(&nullRef), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New(cls, LENGTH_5, nullRef, &array), ANI_OK);
    ASSERT_NE(array, nullptr);
    ASSERT_EQ(env_->FixedArray_GetLength(array, &size), ANI_OK);
    ASSERT_EQ(size, LENGTH_5);

    // Test creating array with initial element
    ani_string str = nullptr;
    const char *utf8String = "test";
    const ani_size stringLength = strlen(utf8String);
    ASSERT_EQ(env_->String_NewUTF8(utf8String, stringLength, &str), ANI_OK);
    ASSERT_NE(str, nullptr);
    ani_fixedarray array2 = nullptr;
    ASSERT_EQ(env_->FixedArray_New(cls, stringLength, str, &array2), ANI_OK);
    ASSERT_NE(array2, nullptr);

    // Verify initial element was set for all elements
    for (ani_size i = 0; i < stringLength; i++) {
        ani_ref element = nullptr;
        ASSERT_EQ(env_->FixedArray_Get(array2, i, &element), ANI_OK);
        ani_size resultSize = 0;
        char utfBuffer[LENGTH_10] = {0};
        ASSERT_EQ(env_->String_GetUTF8SubString(reinterpret_cast<ani_string>(element), ZERO, stringLength, utfBuffer,
                                                sizeof(utfBuffer), &resultSize),
                  ANI_OK);
        ASSERT_STREQ(utfBuffer, utf8String);
    }
}

TEST_F(FixedArrayNewTest, NewObjectArrayTest2)
{
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("std.core.String", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_fixedarray array1 = nullptr;
    ani_fixedarray array2 = nullptr;
    ani_fixedarray array3 = nullptr;
    ani_size size = 0;

    ani_ref undefinedRef = nullptr;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New(cls, MINI_LENGTH, undefinedRef, &array1), ANI_OK);
    ASSERT_NE(array1, nullptr);
    ASSERT_EQ(env_->FixedArray_GetLength(array1, &size), ANI_OK);
    ASSERT_EQ(size, MINI_LENGTH);

    ASSERT_EQ(env_->FixedArray_New(cls, MID_LENGTH, undefinedRef, &array2), ANI_OK);
    ASSERT_NE(array2, nullptr);
    ASSERT_EQ(env_->FixedArray_GetLength(array2, &size), ANI_OK);
    ASSERT_EQ(size, MID_LENGTH);

    ASSERT_EQ(env_->FixedArray_New(cls, BIG_LENGTH, undefinedRef, &array3), ANI_OK);
    ASSERT_NE(array3, nullptr);
    ASSERT_EQ(env_->FixedArray_GetLength(array3, &size), ANI_OK);
    ASSERT_EQ(size, BIG_LENGTH);
}

TEST_F(FixedArrayNewTest, NewObjectArrayTest3)
{
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("std.core.String", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_ref undefinedRef = nullptr;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ani_fixedarray array = nullptr;
        ASSERT_EQ(env_->FixedArray_New(cls, LENGTH_5, undefinedRef, &array), ANI_OK);
        ASSERT_NE(array, nullptr);
    }
}

TEST_F(FixedArrayNewTest, NewObjectArrayTest4)
{
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("std.core.String", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_string str = nullptr;
    auto status = env_->String_NewUTF8("", 0U, &str);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(str, nullptr);

    const ani_size maxNum = std::numeric_limits<uint32_t>::max();
    ani_fixedarray array1 = nullptr;
    ani_ref undefinedRef = nullptr;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New(cls, ZERO, undefinedRef, &array1), ANI_OK);
    ASSERT_NE(array1, nullptr);

    ani_fixedarray array2 = nullptr;
    ASSERT_EQ(env_->FixedArray_New(cls, maxNum, undefinedRef, &array2), ANI_OUT_OF_MEMORY);

    ani_fixedarray array3 = nullptr;
    ASSERT_EQ(env_->FixedArray_New(cls, ZERO, str, &array3), ANI_PENDING_ERROR);

    ani_fixedarray array4 = nullptr;
    ASSERT_EQ(env_->FixedArray_New(cls, maxNum, str, &array4), ANI_PENDING_ERROR);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
