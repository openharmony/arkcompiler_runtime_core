/*
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

#include "utils/math_helpers.h"

#include <cmath>
#include <gtest/gtest.h>

namespace ark::helpers::math::test {

// NOLINTBEGIN(readability-magic-numbers,hicpp-signed-bitwise)

TEST(MathHelpers, GetIntLog2)
{
    for (size_t i = 1; i < 32U; i++) {
        uint64_t val = 1U << i;
        EXPECT_EQ(GetIntLog2(val), i);
        EXPECT_EQ(GetIntLog2(val), log2(static_cast<double>(val)));
    }

    for (size_t i = 1; i < 32U; i++) {
        uint64_t val = (1U << i) + (i == 31U ? -1L : 1U);
        (void)val;
#ifndef NDEBUG
        EXPECT_DEATH_IF_SUPPORTED(GetIntLog2(val), "");
#endif
    }
}

TEST(MathHelpers, IsPowerOfTwo)
{
    EXPECT_TRUE(IsPowerOfTwo(1U));
    EXPECT_TRUE(IsPowerOfTwo(2U));
    EXPECT_TRUE(IsPowerOfTwo(4U));
    EXPECT_TRUE(IsPowerOfTwo(64U));
    EXPECT_TRUE(IsPowerOfTwo(1024U));
    EXPECT_TRUE(IsPowerOfTwo(2048U));
    EXPECT_FALSE(IsPowerOfTwo(3U));
    EXPECT_FALSE(IsPowerOfTwo(63U));
    EXPECT_FALSE(IsPowerOfTwo(65U));
    EXPECT_FALSE(IsPowerOfTwo(100U));
}

TEST(MathHelpers, GetPowerOfTwoValue32)
{
    for (int i = 0; i <= 1; i++) {
        EXPECT_EQ(GetPowerOfTwoValue32(i), 1U);
    }
    for (int i = 2; i <= 2; i++) {
        EXPECT_EQ(GetPowerOfTwoValue32(i), 2U);
    }
    for (int i = 9; i <= 16; i++) {
        EXPECT_EQ(GetPowerOfTwoValue32(i), 16U);
    }
    for (int i = 33; i <= 64; i++) {
        EXPECT_EQ(GetPowerOfTwoValue32(i), 64U);
    }
    for (int i = 1025; i <= 2048; i++) {
        EXPECT_EQ(GetPowerOfTwoValue32(i), 2048U);
    }
}

// NOLINTEND(readability-magic-numbers,hicpp-signed-bitwise)

}  // namespace ark::helpers::math::test
