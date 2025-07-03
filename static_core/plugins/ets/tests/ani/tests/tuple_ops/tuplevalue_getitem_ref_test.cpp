/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ani_gtest_tuple_ops.h"

namespace ark::ets::ani::testing {

class TupleValueGetItemRefTest : public AniGTestTupleOps {
public:
    static constexpr ani_size ARRAY_SIZE = 5;
    static constexpr ani_double EXPECTED_RESULT_1 = 1.0;
    static constexpr ani_double EXPECTED_RESULT_2 = 2.0;
    static constexpr ani_double EXPECTED_RESULT_3 = 3.0;
    static constexpr ani_double EXPECTED_RESULT_4 = 4.0;
    static constexpr ani_double EXPECTED_RESULT_5 = 5.0;
    static constexpr ani_size STRING_SIZE = 6;

protected:
    void CompareStringWithRef(std::string_view expectedElem, ani_ref result)
    {
        auto internalStr = reinterpret_cast<ani_string>(result);
        std::array<char, ARRAY_SIZE + 1> elem {};
        ani_size copiedSize = 0;
        ASSERT_EQ(
            env_->String_GetUTF8SubString(internalStr, 0U, elem.size() - 1, elem.data(), elem.size(), &copiedSize),
            ANI_OK);
        ASSERT_EQ(copiedSize, expectedElem.size());
        ASSERT_STREQ(elem.data(), expectedElem.data());
    }

    void CheckStringValue(ani_tuple_value tupleValue, const ani_size index, const std::string_view &expectedStrView)
    {
        ani_ref result {};
        ASSERT_EQ(env_->TupleValue_GetItem_Ref(tupleValue, index, &result), ANI_OK);
        auto internalStr = reinterpret_cast<ani_string>(result);
        std::string stdString {};
        GetStdString(internalStr, stdString);
        ASSERT_STREQ(stdString.c_str(), expectedStrView.data());
    }

    void CheckDoubleValue(ani_tuple_value tupleValue, const ani_size index, const std::array<double, 5U> &array)
    {
        ani_ref result {};
        ASSERT_EQ(env_->TupleValue_GetItem_Ref(tupleValue, index, &result), ANI_OK);
        auto internalArr = reinterpret_cast<ani_array_double>(result);

        std::vector<double> elem(array.size());
        ASSERT_EQ(env_->Array_GetRegion_Double(internalArr, 0, elem.size(), elem.data()), ANI_OK);

        for (size_t idx = 0; idx < array.size(); ++idx) {
            ASSERT_EQ(elem[idx], array[idx]);
        }
    }
};

TEST_F(TupleValueGetItemRefTest, tupleValueGetItemRef)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_ref_test", "getReferenceTuple");
    ani_ref result {};
    ASSERT_EQ(env_->TupleValue_GetItem_Ref(tuple, 0U, &result), ANI_OK);
    constexpr std::string_view EXPECTED_ELEM("Hello");
    auto internalStr = reinterpret_cast<ani_string>(result);
    std::array<char, EXPECTED_ELEM.size() + 1> elem {};
    ani_size copiedSize = 0;
    ASSERT_EQ(env_->String_GetUTF8SubString(internalStr, 0U, elem.size() - 1, elem.data(), elem.size(), &copiedSize),
              ANI_OK);
    ASSERT_EQ(copiedSize, EXPECTED_ELEM.size());
    ASSERT_STREQ(elem.data(), EXPECTED_ELEM.data());
}

TEST_F(TupleValueGetItemRefTest, tupleValueGetItemRefNullEnv)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_ref_test", "getReferenceTuple");
    ani_ref result {};
    ASSERT_EQ(env_->c_api->TupleValue_GetItem_Ref(nullptr, tuple, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemRefTest, tupleValueGetItemRefNullTuple)
{
    ani_ref result {};
    ASSERT_EQ(env_->TupleValue_GetItem_Ref(nullptr, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemRefTest, tupleValueGetItemRefIndexOutOfRange1)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_ref_test", "getReferenceTuple");
    ani_ref result {};
    ASSERT_EQ(env_->TupleValue_GetItem_Ref(tuple, 4U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemRefTest, tupleValueGetItemRefIndexOutOfRange2)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_ref_test", "getReferenceTuple");
    ani_ref result {};
    ASSERT_EQ(env_->TupleValue_GetItem_Ref(tuple, -1U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemRefTest, tupleValueGetItemRefNullResult)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_ref_test", "getReferenceTuple");
    ASSERT_EQ(env_->TupleValue_GetItem_Ref(tuple, 0U, nullptr), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemRefTest, tupleValueGetItemIntIndexOutOfRange3)
{
    const ani_size maxNum = std::numeric_limits<ani_size>::max();
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_ref_test", "getReferenceTuple");
    ani_ref result {};
    ASSERT_EQ(env_->TupleValue_GetItem_Ref(tuple, maxNum, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemRefTest, tupleValueGetItemIntCompositeScene)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_ref_test", "getReferenceTuple");

    const std::array<ani_double, 5U> expectedArrayValues = {EXPECTED_RESULT_1, EXPECTED_RESULT_2, EXPECTED_RESULT_3,
                                                            EXPECTED_RESULT_4, EXPECTED_RESULT_5};

    ani_ref result {};
    ASSERT_EQ(env_->TupleValue_GetItem_Ref(tuple, 0U, &result), ANI_OK);
    CompareStringWithRef("Hello", result);

    ASSERT_EQ(env_->TupleValue_GetItem_Ref(tuple, 1U, &result), ANI_OK);
    auto internalArray = reinterpret_cast<ani_array_double>(result);
    std::array<ani_double, 5U> nativeBuffer = {0.0};
    ASSERT_EQ(env_->Array_GetRegion_Double(internalArray, 0, 5U, nativeBuffer.data()), ANI_OK);
    for (size_t i = 0; i < expectedArrayValues.size(); ++i) {
        ASSERT_EQ(nativeBuffer[i], expectedArrayValues[i]);
    }

    ASSERT_EQ(env_->TupleValue_GetItem_Ref(tuple, 2U, &result), ANI_OK);
    CompareStringWithRef("world", result);
}

TEST_F(TupleValueGetItemRefTest, tupleValueGetItemRefRepeat)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_ref_test", "getReferenceTuple");
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_ref result {};
        ASSERT_EQ(env_->TupleValue_GetItem_Ref(tuple, 0U, &result), ANI_OK);
        constexpr std::string_view EXPECTED_ELEM("Hello");
        auto internalStr = reinterpret_cast<ani_string>(result);
        std::array<char, EXPECTED_ELEM.size() + 1> elem {};
        ani_size copiedSize = 0;
        ASSERT_EQ(
            env_->String_GetUTF8SubString(internalStr, 0U, elem.size() - 1, elem.data(), elem.size(), &copiedSize),
            ANI_OK);
        ASSERT_EQ(copiedSize, EXPECTED_ELEM.size());
        ASSERT_STREQ(elem.data(), EXPECTED_ELEM.data());
    }
}

TEST_F(TupleValueGetItemRefTest, referenceTuple)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_ref_test", "getReferenceTuple");
    constexpr std::string_view EXPECTED_STR_VIEW1("Hello");
    constexpr std::string_view EXPECTED_STR_VIEW2("world");
    constexpr std::array<double, 5U> EXPECTED_ARRAY = {1, 2, 3, 4, 5};

    CheckStringValue(tuple, 0U, EXPECTED_STR_VIEW1);
    CheckDoubleValue(tuple, 1U, EXPECTED_ARRAY);
    CheckStringValue(tuple, 2U, EXPECTED_STR_VIEW2);

    ani_ref result {};
    ASSERT_EQ(env_->TupleValue_GetItem_Ref(tuple, 0U, &result), ANI_OK);
    auto internalStr = reinterpret_cast<ani_string>(result);
    const ani_size maxNum = std::numeric_limits<ani_size>::max();
    ASSERT_EQ(env_->TupleValue_SetItem_Ref(tuple, maxNum, internalStr), ANI_OUT_OF_RANGE);

    ani_string string {};
    constexpr std::string_view STR_VIEW1("study");
    ASSERT_EQ(env_->String_NewUTF8(STR_VIEW1.data(), STRING_SIZE, &string), ANI_OK);
    ASSERT_EQ(env_->TupleValue_SetItem_Ref(tuple, 0U, string), ANI_OK);
    CheckStringValue(tuple, 0U, STR_VIEW1);

    const auto array =
        static_cast<ani_array_double>(CallEtsFunction<ani_ref>("tuplevalue_getitem_ref_test", "getArray"));
    ASSERT_EQ(env_->TupleValue_SetItem_Ref(tuple, 1U, array), ANI_OK);

    constexpr std::array<double, 5U> EXPECTED_DOUBLE_ARRAY = {100.1, 200.2, 300.3, 400.4, 500.5};
    CheckDoubleValue(tuple, 1U, EXPECTED_DOUBLE_ARRAY);

    ani_string string2 {};
    constexpr std::string_view STR_VIEW2("power");
    ASSERT_EQ(env_->String_NewUTF8(STR_VIEW2.data(), STRING_SIZE, &string2), ANI_OK);
    ASSERT_EQ(env_->TupleValue_SetItem_Ref(tuple, 2U, string2), ANI_OK);
    CheckStringValue(tuple, 2U, STR_VIEW2);
}
}  // namespace ark::ets::ani::testing
