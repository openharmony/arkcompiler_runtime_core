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

#include "ani_gtest_array_ops.h"
#include <iostream>

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ArraySetRefTest : public AniGTestArrayOps {};

// ninja ani_test_array_setref_gtests
TEST_F(ArraySetRefTest, SetRefErrorTests)
{
    ani_array_ref array = nullptr;
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("Lstd/core/String;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(env_->Array_New_Ref(cls, LENGTH_3, nullptr, &array), ANI_OK);
    ani_ref ref = nullptr;
    const ani_size index = 0;
    const ani_size invalidIndex = 5;
    ASSERT_EQ(env_->Array_Set_Ref(nullptr, index, ref), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Array_Set_Ref(array, invalidIndex, ref), ANI_OUT_OF_RANGE);
    auto num = static_cast<ani_ref>(CallEtsFunction<ani_ref>("array_set_ref_test", "GetNumber"));
    // Can set any object as a type
    ASSERT_EQ(env_->Array_Set_Ref(array, 0, num), ANI_OK);
}

TEST_F(ArraySetRefTest, FixedSetRefErrorTests)
{
    ani_array_ref array = nullptr;
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("Lstd/core/String;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(env_->FixedArray_New_Ref(cls, LENGTH_3, nullptr, &array), ANI_OK);
    ani_ref ref = nullptr;
    const ani_size index = 0;
    const ani_size invalidIndex = 5;
    ASSERT_EQ(env_->Array_Set_Ref(nullptr, index, ref), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Array_Set_Ref(array, invalidIndex, ref), ANI_OUT_OF_RANGE);
    auto num = static_cast<ani_ref>(CallEtsFunction<ani_ref>("array_set_ref_test", "GetNumber"));
    ASSERT_EQ(env_->Array_Set_Ref(array, 0, num), ANI_INVALID_TYPE);
}

TEST_F(ArraySetRefTest, SetRefOkTests)
{
    auto array = static_cast<ani_array_ref>(CallEtsFunction<ani_ref>("array_set_ref_test", "GetArray"));

    auto newValue1 = static_cast<ani_ref>(CallEtsFunction<ani_ref>("array_set_ref_test", "GetNewString1"));
    const ani_size index1 = 0;
    ASSERT_EQ(env_->Array_Set_Ref(array, index1, newValue1), ANI_OK);

    auto newValue2 = static_cast<ani_ref>(CallEtsFunction<ani_ref>("array_set_ref_test", "GetNewString2"));
    const ani_size index2 = 2;
    ASSERT_EQ(env_->Array_Set_Ref(array, index2, newValue2), ANI_OK);

    ani_boolean result =
        static_cast<ani_boolean>(CallEtsFunction<ani_boolean>("array_set_ref_test", "CheckArray", array));
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(ArraySetRefTest, SetRefErrorValueToArrayTest)
{
    ani_array_ref array = nullptr;
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("Lstd/core/String;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(env_->Array_New_Ref(cls, LENGTH_3, nullptr, &array), ANI_OK);

    const ani_size errorIndex = -1;
    ASSERT_EQ(env_->Array_Set_Ref(array, errorIndex, nullptr), ANI_OUT_OF_RANGE);
}

TEST_F(ArraySetRefTest, FixedSetRefErrorValueToArrayTest)
{
    ani_array_ref array = nullptr;
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("Lstd/core/String;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(env_->FixedArray_New_Ref(cls, LENGTH_3, nullptr, &array), ANI_OK);

    const ani_size errorIndex = -1;
    ASSERT_EQ(env_->Array_Set_Ref(array, errorIndex, nullptr), ANI_OUT_OF_RANGE);
}

TEST_F(ArraySetRefTest, SetGetUnionToArrayTest)
{
    ani_array_ref array = nullptr;
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("Lstd/core/String;", &cls), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New_Ref(cls, LENGTH_3, nullptr, &array), ANI_OK);

    auto newValue1 = static_cast<ani_ref>(CallEtsFunction<ani_ref>("array_set_ref_test", "GetNewString1"));
    const ani_size index1 = 1;
    ASSERT_EQ(env_->Array_Set_Ref(array, index1, newValue1), ANI_OK);
    ani_ref ref1 = nullptr;
    ASSERT_EQ(env_->Array_Get_Ref(array, index1, &ref1), ANI_OK);

    std::string result {};
    GetStdString(static_cast<ani_string>(ref1), result);
    ASSERT_STREQ(result.c_str(), "New String 1!");

    for (ani_size i = 0; i < LENGTH_5; i++) {
        ASSERT_EQ(env_->Array_Set_Ref(array, index1, newValue1), ANI_OK);
    }
    ASSERT_EQ(env_->Array_Get_Ref(array, index1, &ref1), ANI_OK);
    GetStdString(static_cast<ani_string>(ref1), result);
    ASSERT_STREQ(result.c_str(), "New String 1!");

    const ani_size index2 = 2;
    auto newValue2 = static_cast<ani_ref>(CallEtsFunction<ani_ref>("array_set_ref_test", "GetNewString2"));
    ASSERT_EQ(env_->Array_Set_Ref(array, index1, newValue1), ANI_OK);
    ASSERT_EQ(env_->Array_Set_Ref(array, index2, newValue2), ANI_OK);
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->Array_Set_Ref(array, index1, newValue2), ANI_OK);
        ASSERT_EQ(env_->Array_Set_Ref(array, index2, newValue1), ANI_OK);
    }
    ASSERT_EQ(env_->Array_Get_Ref(array, index1, &ref1), ANI_OK);
    ani_ref ref2 = nullptr;
    std::string result2 {};
    ASSERT_EQ(env_->Array_Get_Ref(array, index2, &ref2), ANI_OK);
    GetStdString(static_cast<ani_string>(ref1), result);
    GetStdString(static_cast<ani_string>(ref2), result2);
    ASSERT_STREQ(result.c_str(), "New String 2!");
    ASSERT_STREQ(result2.c_str(), "New String 1!");

    ASSERT_EQ(env_->Array_Set_Ref(array, index1, newValue1), ANI_OK);
    for (ani_size i = 0; i < LENGTH_5; i++) {
        ASSERT_EQ(env_->Array_Get_Ref(array, index1, &ref1), ANI_OK);
        GetStdString(static_cast<ani_string>(ref1), result);
        ASSERT_STREQ(result.c_str(), "New String 1!");
    }
}

TEST_F(ArraySetRefTest, SetGetStabilityToArrayTest)
{
    ani_array_ref array = nullptr;
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("Lstd/core/String;", &cls), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New_Ref(cls, LENGTH_3, nullptr, &array), ANI_OK);
    ani_ref ref1 = nullptr;
    auto newValue1 = static_cast<ani_ref>(CallEtsFunction<ani_ref>("array_set_ref_test", "GetNewString1"));
    const ani_size index1 = 1;
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->Array_Set_Ref(array, index1, newValue1), ANI_OK);
    }
    ASSERT_EQ(env_->Array_Get_Ref(array, index1, &ref1), ANI_OK);
    std::string result {};
    GetStdString(static_cast<ani_string>(ref1), result);
    ASSERT_STREQ(result.c_str(), "New String 1!");

    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->Array_Set_Ref(array, index1, newValue1), ANI_OK);
    }
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->Array_Get_Ref(array, index1, &ref1), ANI_OK);
    }
    GetStdString(static_cast<ani_string>(ref1), result);
    ASSERT_STREQ(result.c_str(), "New String 1!");

    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->Array_Set_Ref(array, index1, newValue1), ANI_OK);
        ASSERT_EQ(env_->Array_Get_Ref(array, index1, &ref1), ANI_OK);
    }
    GetStdString(static_cast<ani_string>(ref1), result);
    ASSERT_STREQ(result.c_str(), "New String 1!");
}

TEST_F(ArraySetRefTest, EscompatGetRegionRefTest)
{
    auto array = static_cast<ani_array_ref>(CallEtsFunction<ani_ref>("array_set_ref_test", "GetEscompatArray"));
    const ani_size index1 = 1;
    const ani_size index2 = 2;
    ani_ref ref1 = nullptr;
    ani_ref ref2 = nullptr;
    ani_boolean isNull;
    ASSERT_EQ(env_->Array_Get_Ref(array, index1, &ref1), ANI_OK);
    ASSERT_EQ(env_->Array_Get_Ref(array, index2, &ref2), ANI_OK);
    ASSERT_EQ(env_->Reference_IsNull(ref1, &isNull), ANI_OK);
    ASSERT_EQ(isNull, ANI_TRUE);
    ASSERT_EQ(env_->Reference_IsNull(ref2, &isNull), ANI_OK);
    ASSERT_EQ(isNull, ANI_FALSE);
}

TEST_F(ArraySetRefTest, EscompatSetRegionRefTest)
{
    auto array = static_cast<ani_array_ref>(CallEtsFunction<ani_ref>("array_set_ref_test", "GetEscompatArray"));

    auto newValue1 = static_cast<ani_ref>(CallEtsFunction<ani_ref>("array_set_ref_test", "GetNewString1"));
    const ani_size index1 = 0;
    ASSERT_EQ(env_->Array_Set_Ref(array, index1, newValue1), ANI_OK);

    auto newValue2 = static_cast<ani_ref>(CallEtsFunction<ani_ref>("array_set_ref_test", "GetNewString2"));
    const ani_size index2 = 2;
    ASSERT_EQ(env_->Array_Set_Ref(array, index2, newValue2), ANI_OK);

    ani_boolean result =
        static_cast<ani_boolean>(CallEtsFunction<ani_boolean>("array_set_ref_test", "CheckEscompatArray", array));
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(ArraySetRefTest, EscompatInvalidRefTest)
{
    auto array = static_cast<ani_array_ref>(CallEtsFunction<ani_ref>("array_set_ref_test", "GetEscompatArray"));

    auto newValue1 = static_cast<ani_ref>(CallEtsFunction<ani_ref>("array_set_ref_test", "GetNewString1"));
    const ani_size index1 = 5;
    ASSERT_EQ(env_->Array_Set_Ref(array, index1, newValue1), ANI_OUT_OF_RANGE);
    ani_ref res;
    ASSERT_EQ(env_->Array_Get_Ref(array, index1, &res), ANI_OUT_OF_RANGE);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
