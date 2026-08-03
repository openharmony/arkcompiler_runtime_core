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
#include <iostream>

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class FixedArraySetTest : public AniGTestFixedArrayOps {};

// ninja ani_test_fixedarray_set_gtests
TEST_F(FixedArraySetTest, SetErrorTests)
{
    ani_fixedarray array = nullptr;
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("std.core.String", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    std::string_view rawString {"1234"};
    ani_string str {};
    ASSERT_EQ(env_->String_NewUTF8(rawString.data(), rawString.length(), &str), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New(cls, LENGTH_3, str, &array), ANI_OK);
    const ani_size index = 0;
    const ani_size invalidIndex = 5;
    ASSERT_EQ(env_->FixedArray_Set(nullptr, index, str), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->FixedArray_Set(array, invalidIndex, str), ANI_OUT_OF_RANGE);
    auto num = static_cast<ani_ref>(CallEtsFunction<ani_ref>("fixedarray_set_test", "getObject"));
    ASSERT_EQ(env_->FixedArray_Set(array, 0, num), ANI_INVALID_TYPE);
}

TEST_F(FixedArraySetTest, SetOkTests)
{
    auto array = static_cast<ani_fixedarray>(CallEtsFunction<ani_ref>("fixedarray_set_test", "getArray"));

    auto newValue1 = static_cast<ani_ref>(CallEtsFunction<ani_ref>("fixedarray_set_test", "getNewString1"));
    const ani_size index1 = 0;
    ASSERT_EQ(env_->FixedArray_Set(array, index1, newValue1), ANI_OK);

    auto newValue2 = static_cast<ani_ref>(CallEtsFunction<ani_ref>("fixedarray_set_test", "getNewString2"));
    const ani_size index2 = 2;
    ASSERT_EQ(env_->FixedArray_Set(array, index2, newValue2), ANI_OK);

    ani_boolean result =
        static_cast<ani_boolean>(CallEtsFunction<ani_boolean>("fixedarray_set_test", "checkArray", array));
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(FixedArraySetTest, SetErrorValueToArrayTest)
{
    ani_fixedarray array = nullptr;
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("std.core.String", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    std::string_view rawString {"1234"};
    ani_string str {};
    ASSERT_EQ(env_->String_NewUTF8(rawString.data(), rawString.length(), &str), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New(cls, LENGTH_3, str, &array), ANI_OK);

    const ani_size errorIndex = -1;
    ASSERT_EQ(env_->FixedArray_Set(array, errorIndex, str), ANI_OUT_OF_RANGE);
}

TEST_F(FixedArraySetTest, SetGetUnionToArrayTest)
{
    ani_fixedarray array = nullptr;
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("std.core.String", &cls), ANI_OK);
    std::string_view rawString {"1234"};
    ani_string str {};
    ASSERT_EQ(env_->String_NewUTF8(rawString.data(), rawString.length(), &str), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New(cls, LENGTH_3, str, &array), ANI_OK);

    auto newValue1 = static_cast<ani_ref>(CallEtsFunction<ani_ref>("fixedarray_set_test", "getNewString1"));
    const ani_size index1 = 1;
    ASSERT_EQ(env_->FixedArray_Set(array, index1, newValue1), ANI_OK);
    ani_ref ref1 = nullptr;
    ASSERT_EQ(env_->FixedArray_Get(array, index1, &ref1), ANI_OK);

    std::string result {};
    GetStdString(static_cast<ani_string>(ref1), result);
    ASSERT_STREQ(result.c_str(), "New String 1!");

    for (ani_size i = 0; i < LENGTH_5; i++) {
        ASSERT_EQ(env_->FixedArray_Set(array, index1, newValue1), ANI_OK);
    }
    ASSERT_EQ(env_->FixedArray_Get(array, index1, &ref1), ANI_OK);
    GetStdString(static_cast<ani_string>(ref1), result);
    ASSERT_STREQ(result.c_str(), "New String 1!");

    const ani_size index2 = 2;
    auto newValue2 = static_cast<ani_ref>(CallEtsFunction<ani_ref>("fixedarray_set_test", "getNewString2"));
    ASSERT_EQ(env_->FixedArray_Set(array, index1, newValue1), ANI_OK);
    ASSERT_EQ(env_->FixedArray_Set(array, index2, newValue2), ANI_OK);
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_Set(array, index1, newValue2), ANI_OK);
        ASSERT_EQ(env_->FixedArray_Set(array, index2, newValue1), ANI_OK);
    }
    ASSERT_EQ(env_->FixedArray_Get(array, index1, &ref1), ANI_OK);
    ani_ref ref2 = nullptr;
    std::string result2 {};
    ASSERT_EQ(env_->FixedArray_Get(array, index2, &ref2), ANI_OK);
    GetStdString(static_cast<ani_string>(ref1), result);
    GetStdString(static_cast<ani_string>(ref2), result2);
    ASSERT_STREQ(result.c_str(), "New String 2!");
    ASSERT_STREQ(result2.c_str(), "New String 1!");

    ASSERT_EQ(env_->FixedArray_Set(array, index1, newValue1), ANI_OK);
    for (ani_size i = 0; i < LENGTH_5; i++) {
        ASSERT_EQ(env_->FixedArray_Get(array, index1, &ref1), ANI_OK);
        GetStdString(static_cast<ani_string>(ref1), result);
        ASSERT_STREQ(result.c_str(), "New String 1!");
    }
}

TEST_F(FixedArraySetTest, SetGetStabilityToArrayTest)
{
    ani_fixedarray array = nullptr;
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("std.core.String", &cls), ANI_OK);
    std::string_view rawString {"1234"};
    ani_string str {};
    ASSERT_EQ(env_->String_NewUTF8(rawString.data(), rawString.length(), &str), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New(cls, LENGTH_3, str, &array), ANI_OK);
    ani_ref ref1 = nullptr;
    auto newValue1 = static_cast<ani_ref>(CallEtsFunction<ani_ref>("fixedarray_set_test", "getNewString1"));
    const ani_size index1 = 1;
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_Set(array, index1, newValue1), ANI_OK);
    }
    ASSERT_EQ(env_->FixedArray_Get(array, index1, &ref1), ANI_OK);
    std::string result {};
    GetStdString(static_cast<ani_string>(ref1), result);
    ASSERT_STREQ(result.c_str(), "New String 1!");

    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_Set(array, index1, newValue1), ANI_OK);
    }
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_Get(array, index1, &ref1), ANI_OK);
    }
    GetStdString(static_cast<ani_string>(ref1), result);
    ASSERT_STREQ(result.c_str(), "New String 1!");

    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_Set(array, index1, newValue1), ANI_OK);
        ASSERT_EQ(env_->FixedArray_Get(array, index1, &ref1), ANI_OK);
    }
    GetStdString(static_cast<ani_string>(ref1), result);
    ASSERT_STREQ(result.c_str(), "New String 1!");
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
