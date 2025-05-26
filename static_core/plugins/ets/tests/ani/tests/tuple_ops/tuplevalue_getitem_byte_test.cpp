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

class TupleValueGetItemByteTest : public AniGTestTupleOps {
public:
    static constexpr ani_byte EXPECTED_RESULT = 125;
};

TEST_F(TupleValueGetItemByteTest, tupleValueGetItemByte)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_byte_test", "getByteTuple");
    ani_byte result = 0;
    ASSERT_EQ(env_->TupleValue_GetItem_Byte(tuple, 0U, &result), ANI_OK);
    ASSERT_EQ(result, EXPECTED_RESULT);
}

TEST_F(TupleValueGetItemByteTest, tupleValueGetItemByteNullEnv)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_byte_test", "getByteTuple");
    ani_byte result = 0;
    ASSERT_EQ(env_->c_api->TupleValue_GetItem_Byte(nullptr, tuple, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemByteTest, tupleValueGetItemByteNullTuple)
{
    ani_byte result = 0;
    ASSERT_EQ(env_->TupleValue_GetItem_Byte(nullptr, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemByteTest, tupleValueGetItemByteIndexOutOfRange1)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_byte_test", "getByteTuple");
    ani_byte result = 0;
    ASSERT_EQ(env_->TupleValue_GetItem_Byte(tuple, 6U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemByteTest, tupleValueGetItemByteIndexOutOfRange2)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_byte_test", "getByteTuple");
    ani_byte result = 0;
    ASSERT_EQ(env_->TupleValue_GetItem_Byte(tuple, -1U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemByteTest, tupleValueGetItemByteNullResult)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_byte_test", "getByteTuple");
    ASSERT_EQ(env_->TupleValue_GetItem_Byte(tuple, 0U, nullptr), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemByteTest, tupleValueGetItemByteRepeat)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_byte_test", "getByteTuple");
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_byte result = 0;
        ASSERT_EQ(env_->TupleValue_GetItem_Byte(tuple, 0U, &result), ANI_OK);
        ASSERT_EQ(result, EXPECTED_RESULT);
    }
}
}  // namespace ark::ets::ani::testing