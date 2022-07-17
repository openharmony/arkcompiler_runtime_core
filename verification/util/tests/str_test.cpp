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

#include <string>

#include "util/str.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

namespace panda::verifier::test {

std::string *Generator()
{
    std::string str = "Generator";
    static std::string cur_str;
    static int str_pos = 0;
    if (str_pos < 0x9) {
        cur_str = str[str_pos];
        str_pos++;
        return &cur_str;
    }
    str_pos = 0;
    return nullptr;
}

TEST_F(VerifierTest, str)
{
    EXPECT_EQ(Join<std::string>(Generator), "G, e, n, e, r, a, t, o, r");
    EXPECT_EQ(Join<std::string>(Generator, "."), "G.e.n.e.r.a.t.o.r");

    EXPECT_EQ(NumToStr(-1456), "-1456");
    EXPECT_EQ(NumToStr(0x1C, 0x10), "1c");
}

}  // namespace panda::verifier::test