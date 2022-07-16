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

#include <vector>

#include "util/int_set.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

namespace panda::verifier::test {

TEST_F(VerifierTest, IntSetBase)
{
    IntSet<size_t> iset;

    EXPECT_EQ(iset.Size(), 0);

    iset.Insert(5);

    EXPECT_EQ(iset.Size(), 1);
    EXPECT_TRUE(iset.Contains(5));
    EXPECT_FALSE(iset.Contains(3));

    std::vector<size_t> v {1, 5, 3};
    iset.Insert(v.begin(), v.begin() + 2);

    EXPECT_EQ(iset.Size(), 2);
    EXPECT_TRUE(iset.Contains(1));
    EXPECT_TRUE(iset.Contains(5));
    EXPECT_FALSE(iset.Contains(3));

    IntSet<size_t> iset1 = iset;
    IntSet<size_t> iset2;
    iset2.Insert(v.begin(), v.end());

    EXPECT_EQ(iset1.Size(), 2);
    EXPECT_TRUE(iset1.Contains(1));
    EXPECT_TRUE(iset1.Contains(5));
    EXPECT_FALSE(iset1.Contains(3));

    EXPECT_EQ(iset2.Size(), 3);
    EXPECT_TRUE(iset2.Contains(1));
    EXPECT_TRUE(iset2.Contains(5));
    EXPECT_TRUE(iset2.Contains(3));

    EXPECT_EQ(iset, iset1);
    EXPECT_NE(iset, iset2);
}

TEST_F(VerifierTest, IntSetInteraction)
{
    std::vector<size_t> v {1, 5, 3};

    IntSet<size_t> iset1;
    IntSet<size_t> iset2;
    iset1.Insert(v.begin(), v.begin() + 2);
    iset1.Insert(8);
    iset2.Insert(v.begin(), v.end());

    EXPECT_NE(iset1, iset2);

    auto iset3 = iset1 & iset2;
    auto iset4 = iset1 | iset2;
    auto iset5 = iset1;
    auto iset6 = iset2;
    iset5 &= iset2;
    iset6 |= iset1;

    EXPECT_EQ(iset3.Size(), 2);
    EXPECT_TRUE(iset3.Contains(1));
    EXPECT_TRUE(iset3.Contains(5));
    EXPECT_FALSE(iset3.Contains(3));
    EXPECT_FALSE(iset3.Contains(8));

    EXPECT_EQ(iset4.Size(), 4);
    EXPECT_TRUE(iset4.Contains(1));
    EXPECT_TRUE(iset4.Contains(5));
    EXPECT_TRUE(iset4.Contains(3));
    EXPECT_TRUE(iset4.Contains(8));

    EXPECT_EQ(iset3, iset5);
    EXPECT_EQ(iset4, iset6);

    auto li = iset1.LazyIntersect(iset2);

    EXPECT_EQ(static_cast<size_t>(li()), 1U);
    EXPECT_EQ(static_cast<size_t>(li()), 5U);
    EXPECT_FALSE(static_cast<bool>(li()));

    EXPECT_EQ(iset3, iset5);

    int res = 0;
    iset1.ForAll([&res](int x) {
        res += x;
        return true;
    });

    EXPECT_EQ(res, 14);

    auto stream = iset1.AsStream();

    EXPECT_EQ(static_cast<size_t>(stream()), 1U);
    EXPECT_EQ(static_cast<size_t>(stream()), 5U);
    EXPECT_EQ(static_cast<size_t>(stream()), 8U);
    EXPECT_FALSE(static_cast<bool>(stream()));
}

}  // namespace panda::verifier::test