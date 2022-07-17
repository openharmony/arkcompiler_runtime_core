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
    Index<int> default_index;
    EXPECT_FALSE(default_index.IsValid());

    default_index = Index<int>(7);
    EXPECT_TRUE(default_index.IsValid());

    int number = default_index;
    EXPECT_EQ(number, 7);

    number = *default_index;
    EXPECT_EQ(number, 7);

    default_index = 5;
    ASSERT_TRUE(default_index.IsValid());
    EXPECT_EQ(static_cast<int>(default_index), 5);

    default_index.Invalidate();
    EXPECT_FALSE(default_index.IsValid());

#ifndef NDEBUG
    EXPECT_DEATH(number = default_index, "");
#endif

    Index<int> default_index1 {4};
    EXPECT_TRUE(default_index1.IsValid());
    EXPECT_EQ(static_cast<int>(default_index1), 4);
    EXPECT_FALSE(default_index == default_index1);
    EXPECT_TRUE(default_index != default_index1);

    default_index = std::move(default_index1);
    ASSERT_TRUE(default_index.IsValid());
    EXPECT_EQ(static_cast<int>(default_index), 4);
    EXPECT_FALSE(default_index1.IsValid());

    default_index1 = default_index;
    ASSERT_TRUE(default_index.IsValid());
    EXPECT_EQ(static_cast<int>(default_index), 4);
    ASSERT_TRUE(default_index1.IsValid());
    EXPECT_EQ(static_cast<int>(default_index1), 4);

    EXPECT_TRUE(static_cast<bool>(default_index));
    default_index.Invalidate();
    EXPECT_FALSE(static_cast<bool>(default_index));

    Index<int, 9> custom_index;
    EXPECT_FALSE(custom_index.IsValid());

#ifndef NDEBUG
    EXPECT_DEATH(custom_index = 9, "");
#endif

    Index<int, 9> custom_index1 {std::move(custom_index)};
    EXPECT_FALSE(custom_index.IsValid());
    EXPECT_FALSE(custom_index1.IsValid());

    custom_index = 5;
    ASSERT_TRUE(custom_index.IsValid());
    EXPECT_EQ(static_cast<int>(custom_index), 5);
    EXPECT_EQ(static_cast<double>(custom_index), 5.0);
}

TEST_F(VerifierTest, index_hash)
{
    std::unordered_set<Index<int, 8>> i_set;  // containers mustn't contain invalid index
    i_set.emplace(5);
    i_set.insert(Index<int, 8> {7});
    EXPECT_EQ(i_set.size(), 2);
    EXPECT_EQ(i_set.count(5), 1);
    EXPECT_EQ(i_set.count(6), 0);
    EXPECT_EQ(i_set.count(7), 1);
}

}  // namespace panda::verifier::test