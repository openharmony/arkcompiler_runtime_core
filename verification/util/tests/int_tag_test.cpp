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

#include "util/int_tag.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

namespace panda::verifier::test {

TEST_F(VerifierTest, IntTag)
{
    using IntTag1 = TagForInt<int, 3, 9>;

    ASSERT_EQ(IntTag1::Size, 7);
    EXPECT_EQ(IntTag1::Bits, 3);
    EXPECT_EQ(IntTag1::GetIndexFor(4), 1);
    EXPECT_EQ(IntTag1::GetValueFor(3), 6);

    using IntTag2 = TagForInt<int, 5, 5>;

    ASSERT_EQ(IntTag2::Size, 1);
    EXPECT_EQ(IntTag2::Bits, 1);
    EXPECT_EQ(IntTag2::GetIndexFor(5), 0);
    EXPECT_EQ(IntTag2::GetValueFor(0), 5);
}

}  // namespace panda::verifier::test