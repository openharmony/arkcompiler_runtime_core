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

class TupleValueGetItemLongTest : public AniGTestTupleOps {
public:
    static constexpr ani_long EXPECTED_RESULT = 300L;
};

TEST_F(TupleValueGetItemLongTest, tupleValueGetItemLong)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_long_test", "getLongTuple");
    ani_long result = 0L;
    ASSERT_EQ(env_->TupleValue_GetItem_Long(tuple, 0U, &result), ANI_OK);
    ASSERT_EQ(result, EXPECTED_RESULT);
}

TEST_F(TupleValueGetItemLongTest, tupleValueGetItemLongNullEnv)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_long_test", "getLongTuple");
    ani_long result = 0L;
    ASSERT_EQ(env_->c_api->TupleValue_GetItem_Long(nullptr, tuple, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemLongTest, tupleValueGetItemLongNullTuple)
{
    ani_long result = 0L;
    ASSERT_EQ(env_->TupleValue_GetItem_Long(nullptr, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemLongTest, tupleValueGetItemLongIndexOutOfRange1)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_long_test", "getLongTuple");
    ani_long result = 0L;
    ASSERT_EQ(env_->TupleValue_GetItem_Long(tuple, 6U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemLongTest, tupleValueGetItemLongIndexOutOfRange2)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_long_test", "getLongTuple");
    ani_long result = 0L;
    ASSERT_EQ(env_->TupleValue_GetItem_Long(tuple, -1U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemLongTest, tupleValueGetItemLongNullResult)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_long_test", "getLongTuple");
    ASSERT_EQ(env_->TupleValue_GetItem_Long(tuple, 0U, nullptr), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemLongTest, tupleValueGetItemLongRepeat)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_long_test", "getLongTuple");
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_long result = 0L;
        ASSERT_EQ(env_->TupleValue_GetItem_Long(tuple, 0U, &result), ANI_OK);
        ASSERT_EQ(result, EXPECTED_RESULT);
    }
}
}  // namespace ark::ets::ani::testing