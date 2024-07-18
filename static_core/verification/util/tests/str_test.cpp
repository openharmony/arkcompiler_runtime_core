/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <string>

#include "util/str.h"

#include "util/tests/verifier_test.h"
#include "libpandabase/utils/utils.h"

#include <gtest/gtest.h>

namespace ark::verifier::test {

std::string *Generator()
{
    std::string str = "Generator";
    static std::string curStr;
    static int strPos = 0;
    // NOLINTNEXTLINE(readability-magic-numbers)
    if (strPos < 0x9) {
        curStr = str[strPos];
        strPos++;
        return &curStr;
    }
    strPos = 0;
    return nullptr;
}

TEST_F(VerifierTest, str)
{
    EXPECT_EQ(Join<std::string>(Generator), "G, e, n, e, r, a, t, o, r");
    EXPECT_EQ(Join<std::string>(Generator, "."), "G.e.n.e.r.a.t.o.r");

    EXPECT_EQ(NumToStr(-1456_I), "-1456");
    EXPECT_EQ(NumToStr(0x1C, 0x10), "1c");
}

}  // namespace ark::verifier::test
