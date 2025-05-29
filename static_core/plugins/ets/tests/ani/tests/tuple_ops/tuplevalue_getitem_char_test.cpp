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

class TupleValueGetItemCharTest : public AniGTestTupleOps {};

TEST_F(TupleValueGetItemCharTest, tupleValueGetItemChar)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_char_test", "getCharTuple");
    ani_char result = '\0';
    ASSERT_EQ(env_->TupleValue_GetItem_Char(tuple, 0U, &result), ANI_OK);
    ASSERT_EQ(result, 'H');
}

TEST_F(TupleValueGetItemCharTest, tupleValueGetItemCharNullEnv)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_char_test", "getCharTuple");
    ani_char result = '\0';
    ASSERT_EQ(env_->c_api->TupleValue_GetItem_Char(nullptr, tuple, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemCharTest, tupleValueGetItemCharReferenceTuple)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_char_test", "getReferenceTuple");
    ani_char result = '\0';
    ASSERT_EQ(env_->TupleValue_GetItem_Char(tuple, 0U, &result), ANI_INVALID_TYPE);
}

TEST_F(TupleValueGetItemCharTest, tupleValueGetItemCharNullTuple)
{
    ani_char result = '\0';
    ASSERT_EQ(env_->TupleValue_GetItem_Char(nullptr, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemCharTest, tupleValueGetItemCharIndexOutOfRange1)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_char_test", "getCharTuple");
    ani_char result = '\0';
    ASSERT_EQ(env_->TupleValue_GetItem_Char(tuple, 6U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemCharTest, tupleValueGetItemCharIndexOutOfRange2)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_char_test", "getCharTuple");
    ani_char result = '\0';
    ASSERT_EQ(env_->TupleValue_GetItem_Char(tuple, -1U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemCharTest, tupleValueGetItemCharNullResult)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_char_test", "getCharTuple");
    ASSERT_EQ(env_->TupleValue_GetItem_Char(tuple, 0U, nullptr), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemCharTest, tupleValueGetItemCharRepeat)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_char_test", "getCharTuple");
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_char result = '\0';
        ASSERT_EQ(env_->TupleValue_GetItem_Char(tuple, 0U, &result), ANI_OK);
        ASSERT_EQ(result, 'H');
    }
}
}  // namespace ark::ets::ani::testing