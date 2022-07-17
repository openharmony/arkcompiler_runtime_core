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

#include "util/abstract_index.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

namespace panda::verifier::test {

namespace {

class Wrap final {
public:
    Wrap() : default_index1 {}, default_index2 {4}
    {
        EXPECT_FALSE(default_index1.IsValid());

        default_index1 = AbstractIndex<int, Wrap>(0x7);
        EXPECT_TRUE(default_index1.IsValid());

        int number = default_index1;
        EXPECT_EQ(number, 0x7);

        number = *default_index1;
        EXPECT_EQ(number, 0x7);

        default_index1 = 0x5;
        EXPECT_TRUE(default_index1.IsValid());
        EXPECT_EQ(static_cast<int>(default_index1), 0x5);

        default_index1.Invalidate();
        EXPECT_FALSE(default_index1.IsValid());

#ifndef NDEBUG
        EXPECT_DEATH(number = default_index1, "");
#endif

        EXPECT_TRUE(default_index2.IsValid());
        EXPECT_EQ(static_cast<int>(default_index2), 0x4);

        default_index1 = std::move(default_index2);
        EXPECT_TRUE(default_index1.IsValid());
        EXPECT_EQ(static_cast<int>(default_index1), 0x4);
        EXPECT_FALSE(default_index2.IsValid());

        default_index2 = default_index1;
        EXPECT_TRUE(default_index1.IsValid());
        EXPECT_EQ(static_cast<int>(default_index1), 0x4);
        EXPECT_TRUE(default_index2.IsValid());
        EXPECT_EQ(static_cast<int>(default_index2), 0x4);

        EXPECT_TRUE(static_cast<bool>(default_index1));
        default_index1.Invalidate();
        EXPECT_FALSE(static_cast<bool>(default_index1));

        default_index1 = 0x4;
        default_index2 = 0x5;

        std::unordered_set<AbstractIndex<int, Wrap>> i_set;  // containers mustn't contain invalid index
        i_set.insert(default_index1);
        i_set.insert(default_index2);
        EXPECT_EQ(i_set.size(), 0x2);
        EXPECT_EQ(i_set.count(0x4), 1);
        EXPECT_EQ(i_set.count(0x5), 1);
        EXPECT_EQ(i_set.count(0x6), 0);
    }

    AbstractIndex<int, Wrap> index1() const
    {
        return default_index1;
    }
    AbstractIndex<int, Wrap> index2() const
    {
        return default_index2;
    }

private:
    AbstractIndex<int, Wrap> default_index1;
    AbstractIndex<int, Wrap> default_index2;
};
}  // namespace

TEST_F(VerifierTest, abstract_index)
{
    Wrap wr;

    AbstractIndex<int, Wrap> index1 = wr.index1();
    AbstractIndex<int, Wrap> index2 = wr.index2();

    EXPECT_TRUE(index1.IsValid());
    EXPECT_TRUE(index2.IsValid());
    EXPECT_FALSE(index1 == index2);
    EXPECT_TRUE(index1 != index2);
    EXPECT_TRUE(index1 < index2);
    EXPECT_TRUE(index1 <= index2);
    EXPECT_TRUE(index1 <= index2);
}

}  // namespace panda::verifier::test