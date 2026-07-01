/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "util/range.h"

#include "util/tests/verifier_test.h"
#include "libarkbase/utils/utils.h"

#include <gtest/gtest.h>
#include <vector>

namespace ark::verifier::test {

TEST_F(VerifierTest, RangeEmptyContainer)
{
    std::vector<int> empty;
    Range<size_t> r(empty);
    EXPECT_FALSE(r.Contains(0_I));
    EXPECT_FALSE(r.Contains(1_I));
    EXPECT_FALSE(r.Contains(100_I));
    EXPECT_EQ(r.Length(), 0U);
}

TEST_F(VerifierTest, RangeNonEmptyContainer)
{
    std::vector<int> items {1_I, 2_I, 3_I};
    Range<size_t> r(items);
    EXPECT_TRUE(r.Contains(0_I));
    EXPECT_TRUE(r.Contains(2_I));
    EXPECT_FALSE(r.Contains(3_I));
    EXPECT_EQ(r.Length(), 3U);
}

}  // namespace ark::verifier::test
