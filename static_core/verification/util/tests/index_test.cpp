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

#include <unordered_set>

#include "util/index.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

namespace panda::verifier::test {

TEST_F(VerifierTest, index)
{
    Index<int> defaultIndex;
    EXPECT_FALSE(defaultIndex.IsValid());

    defaultIndex = Index<int>(7);
    EXPECT_TRUE(defaultIndex.IsValid());

    int number = defaultIndex;
    EXPECT_EQ(number, 7);

    number = *defaultIndex;
    EXPECT_EQ(number, 7);

    defaultIndex = 5;
    ASSERT_TRUE(defaultIndex.IsValid());
    EXPECT_EQ(static_cast<int>(defaultIndex), 5);

    defaultIndex.Invalidate();
    EXPECT_FALSE(defaultIndex.IsValid());

#ifndef NDEBUG
    EXPECT_DEATH(number = defaultIndex, "");
#endif

    Index<int> defaultIndex1 {4};
    EXPECT_TRUE(defaultIndex1.IsValid());
    EXPECT_EQ(static_cast<int>(defaultIndex1), 4);
    EXPECT_FALSE(defaultIndex == defaultIndex1);
    EXPECT_TRUE(defaultIndex != defaultIndex1);

    defaultIndex = std::move(defaultIndex1);
    ASSERT_TRUE(defaultIndex.IsValid());
    EXPECT_EQ(static_cast<int>(defaultIndex), 4);
    // NOLINTNEXTLINE(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
    EXPECT_FALSE(defaultIndex1.IsValid());

    defaultIndex1 = defaultIndex;
    ASSERT_TRUE(defaultIndex.IsValid());
    EXPECT_EQ(static_cast<int>(defaultIndex), 4);
    ASSERT_TRUE(defaultIndex1.IsValid());
    EXPECT_EQ(static_cast<int>(defaultIndex1), 4);

    EXPECT_TRUE(static_cast<bool>(defaultIndex));
    defaultIndex.Invalidate();
    EXPECT_FALSE(static_cast<bool>(defaultIndex));

    // NOLINTNEXTLINE(readability-magic-numbers)
    Index<int, 9> customIndex;
    EXPECT_FALSE(customIndex.IsValid());

#ifndef NDEBUG
    // NOLINTNEXTLINE(readability-magic-numbers)
    EXPECT_DEATH(customIndex = 9, "");
#endif

    // NOLINTNEXTLINE(readability-magic-numbers)
    Index<int, 9> customIndex1 {std::move(customIndex)};
    // NOLINTNEXTLINE(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
    EXPECT_FALSE(customIndex.IsValid());
    EXPECT_FALSE(customIndex1.IsValid());

    customIndex = 5;
    ASSERT_TRUE(customIndex.IsValid());
    EXPECT_EQ(static_cast<int>(customIndex), 5);
    EXPECT_EQ(static_cast<double>(customIndex), 5.0);
}

TEST_F(VerifierTest, index_hash)
{
    std::unordered_set<Index<int, 8>> iSet;  // containers mustn't contain invalid index
    iSet.emplace(5);
    iSet.insert(Index<int, 8> {7});
    EXPECT_EQ(iSet.size(), 2);
    EXPECT_EQ(iSet.count(5), 1);
    EXPECT_EQ(iSet.count(6), 0);
    EXPECT_EQ(iSet.count(7), 1);
}

}  // namespace panda::verifier::test