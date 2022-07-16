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

#include "util/saturated_enum.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

namespace panda::verifier::test {

enum class Numbers { ONE, TWO, THREE, FOUR, FIVE };

TEST_F(VerifierTest, saturated_enum)
{
    SaturatedEnum<Numbers, Numbers::FOUR, Numbers::THREE, Numbers::TWO, Numbers::ONE> s_enum;
    EXPECT_EQ(s_enum, Numbers::ONE);

#ifndef NDEBUG
    EXPECT_DEATH(s_enum[Numbers::FIVE], "");
#endif

    EXPECT_FALSE(s_enum[Numbers::TWO]);
    EXPECT_TRUE(s_enum[Numbers::ONE]);

    s_enum |= Numbers::THREE;
    EXPECT_EQ(s_enum, Numbers::THREE);
    EXPECT_FALSE(s_enum[Numbers::FOUR]);
    EXPECT_TRUE(s_enum[Numbers::TWO]);

    int i = 0;
    s_enum.EnumerateValues([&i](Numbers) {
        i++;
        return true;
    });
    EXPECT_EQ(i, 3);

    i = 0;
    s_enum.EnumerateValues([&i](Numbers en) {
        i++;
        if (en == Numbers::TWO)
            return false;
        return true;
    });
    EXPECT_EQ(i, 2);
}

}  // namespace panda::verifier::test