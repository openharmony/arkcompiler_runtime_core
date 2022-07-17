/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "util/shifted_vector.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

namespace panda::verifier::test {

TEST_F(VerifierTest, shifted_vector)
{
    ShiftedVector<2, int> shift_vec {5};
    ASSERT_EQ(shift_vec.begin_index(), -2);
    ASSERT_EQ(shift_vec.end_index(), 3);
    EXPECT_TRUE(shift_vec.InValidRange(-1));
    EXPECT_FALSE(shift_vec.InValidRange(5));

    shift_vec[0] = 7;
    shift_vec.at(1) = 8;
    EXPECT_EQ(shift_vec.at(0), 7);
    EXPECT_EQ(shift_vec[1], 8);

    shift_vec.ExtendToInclude(5);
    ASSERT_EQ(shift_vec.begin_index(), -2);
    ASSERT_EQ(shift_vec.end_index(), 6);
    EXPECT_TRUE(shift_vec.InValidRange(-1));
    EXPECT_TRUE(shift_vec.InValidRange(5));
    EXPECT_EQ(shift_vec.at(0), 7);
    EXPECT_EQ(shift_vec[1], 8);
    shift_vec[4] = 4;
    EXPECT_EQ(shift_vec.at(4), 4);
}

}  // namespace panda::verifier::test