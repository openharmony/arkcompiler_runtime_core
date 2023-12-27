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
    ShiftedVector<2, int> shiftVec {5};
    ASSERT_EQ(shiftVec.BeginIndex(), -2);
    ASSERT_EQ(shiftVec.EndIndex(), 3);
    EXPECT_TRUE(shiftVec.InValidRange(-1));
    EXPECT_FALSE(shiftVec.InValidRange(5));

    shiftVec[0] = 7;
    shiftVec.At(1) = 8;
    EXPECT_EQ(shiftVec.At(0), 7);
    EXPECT_EQ(shiftVec[1], 8);

    shiftVec.ExtendToInclude(5);
    ASSERT_EQ(shiftVec.BeginIndex(), -2);
    ASSERT_EQ(shiftVec.EndIndex(), 6);
    EXPECT_TRUE(shiftVec.InValidRange(-1));
    EXPECT_TRUE(shiftVec.InValidRange(5));
    EXPECT_EQ(shiftVec.At(0), 7);
    EXPECT_EQ(shiftVec[1], 8);
    shiftVec[4] = 4;
    EXPECT_EQ(shiftVec.At(4), 4);
}

}  // namespace panda::verifier::test