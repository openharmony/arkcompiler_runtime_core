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

class TupleOpsTest : public AniGTestTupleOps {};

TEST_F(TupleOpsTest, getLengthOfEmptyTuple)
{
    auto tuple = GetTupleWithCheck("tuple", "getEmptyTuple");

    ani_size length;
    ASSERT_EQ(env_->TupleValue_GetNumberOfItems(tuple, &length), ANI_OK);
    ASSERT_EQ(length, 0U);
}

TEST_F(TupleOpsTest, getLengthOfValidTuple)
{
    auto tuple = GetTupleWithCheck("tuple", "getTestPrimitiveTuple");

    ani_size length;
    ASSERT_EQ(env_->TupleValue_GetNumberOfItems(tuple, &length), ANI_OK);
    ASSERT_EQ(length, 8U);
}

TEST_F(TupleOpsTest, emptyTuple)
{
    auto tuple = GetTupleWithCheck("tuple", "getEmptyTuple");

    ani_boolean result;
    ASSERT_EQ(env_->TupleValue_GetItem_Boolean(tuple, 0, &result), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->TupleValue_SetItem_Boolean(tuple, 0, false), ANI_OUT_OF_RANGE);
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PRIMITIVE_GET_SET_TEST_CASE(calltype, anitype, idx, expectedValue, updateValue)  \
    do {                                                                                 \
        anitype _result;                                                                 \
        ASSERT_EQ(env_->TupleValue_GetItem_##calltype(tuple, idx, &_result), ANI_OK);    \
        ASSERT_EQ(_result, expectedValue);                                               \
                                                                                         \
        ASSERT_EQ(env_->TupleValue_SetItem_##calltype(tuple, idx, updateValue), ANI_OK); \
        ASSERT_EQ(env_->TupleValue_GetItem_##calltype(tuple, idx, &_result), ANI_OK);    \
        ASSERT_EQ(_result, updateValue);                                                 \
    } while (false)

TEST_F(TupleOpsTest, primitiveTuple)
{
    auto tuple = GetTupleWithCheck("tuple", "getTestPrimitiveTuple");

    // clang-format off
    PRIMITIVE_GET_SET_TEST_CASE(Boolean, ani_boolean, 0U, true,    false);
    PRIMITIVE_GET_SET_TEST_CASE(Char,    ani_char,    1U, 'x',     'y');
    PRIMITIVE_GET_SET_TEST_CASE(Byte,    ani_byte,    2U, 127U,    100U);
    PRIMITIVE_GET_SET_TEST_CASE(Short,   ani_short,   3U, 300U,    100U);
    PRIMITIVE_GET_SET_TEST_CASE(Int,     ani_int,     4U, 500U,   -100L);
    PRIMITIVE_GET_SET_TEST_CASE(Long,    ani_long,    5U, 600U,   -100L);
    PRIMITIVE_GET_SET_TEST_CASE(Float,   ani_float,   6U, 100.0F, -100.0F);
    PRIMITIVE_GET_SET_TEST_CASE(Double,  ani_double,  7U, 400.0F, -100.0F);
    // clang-format on
}

TEST_F(TupleOpsTest, referenceTuple)
{
    auto tuple = GetTupleWithCheck("tuple", "getReferenceTuple");

    ani_ref result;
    ASSERT_EQ(env_->TupleValue_GetItem_Ref(tuple, 0U, &result), ANI_OK);

    constexpr std::string_view EXPECTED_ELEM1("Hello");
    constexpr std::string_view EXPECTED_ELEM3("world");

    auto internalStr = reinterpret_cast<ani_string>(result);
    std::array<char, EXPECTED_ELEM1.size() + 1> elem1 {};

    ani_size copiedSize;
    ASSERT_EQ(env_->String_GetUTF8SubString(internalStr, 0U, elem1.size() - 1, elem1.data(), elem1.size(), &copiedSize),
              ANI_OK);
    ASSERT_EQ(copiedSize, EXPECTED_ELEM1.size());

    ASSERT_STREQ(elem1.data(), EXPECTED_ELEM1.data());

    ASSERT_EQ(env_->TupleValue_GetItem_Ref(tuple, 1U, &result), ANI_OK);
    auto internalArr = reinterpret_cast<ani_array_double>(result);

    std::array<double, 5U> expectedArr = {1, 2, 3, 4, 5};
    std::array<double, expectedArr.size()> elem2 {};

    ASSERT_EQ(env_->Array_GetRegion_Double(internalArr, 0, elem2.size(), elem2.data()), ANI_OK);

    for (size_t idx = 0; idx < expectedArr.size(); ++idx) {
        ASSERT_EQ(elem2[idx], expectedArr[idx]);
    }

    ASSERT_EQ(env_->TupleValue_GetItem_Ref(tuple, 2U, &result), ANI_OK);
    internalStr = reinterpret_cast<ani_string>(result);
    std::array<char, EXPECTED_ELEM3.size() + 1> elem3 {};

    ASSERT_EQ(env_->String_GetUTF8SubString(internalStr, 0U, elem3.size() - 1, elem3.data(), elem3.size(), &copiedSize),
              ANI_OK);
    ASSERT_EQ(copiedSize, EXPECTED_ELEM3.size());

    ASSERT_STREQ(elem3.data(), EXPECTED_ELEM3.data());
}

TEST_F(TupleOpsTest, identicalTuple)
{
    auto tuple = GetTupleWithCheck("tuple", "getCharTuple");

    ani_size length;
    ASSERT_EQ(env_->TupleValue_GetNumberOfItems(tuple, &length), ANI_OK);

    std::array<char, 5U> expectedTuple = {'H', 'e', 'l', 'l', 'o'};

    ani_char currentChar;
    for (ani_size idx = 0; idx < length; ++idx) {
        env_->TupleValue_GetItem_Char(tuple, idx, &currentChar);
        ASSERT_EQ(currentChar, expectedTuple[idx]);
    }
}

TEST_F(TupleOpsTest, DISABLED_incorrectElementType)
{
    auto tuple = GetTupleWithCheck("tuple", "getCharTuple");
    ani_boolean booleanValue = ANI_FALSE;
    ASSERT_EQ(env_->TupleValue_GetItem_Boolean(tuple, 0U, &booleanValue), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->TupleValue_SetItem_Boolean(tuple, 0U, booleanValue), ANI_INVALID_TYPE);

    auto boxedPrimitivesTuple = GetTupleWithCheck("tuple", "getTestPrimitiveTuple");
    ani_char charValue = ANI_FALSE;
    ASSERT_EQ(env_->TupleValue_GetItem_Char(boxedPrimitivesTuple, 0U, &charValue), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->TupleValue_SetItem_Char(boxedPrimitivesTuple, 0U, charValue), ANI_INVALID_TYPE);
}

}  // namespace ark::ets::ani::testing
