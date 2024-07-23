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

#include "util/callable.h"

#include "util/tests/verifier_test.h"
#include "libpandabase/utils/utils.h"

#include <gtest/gtest.h>

namespace ark::verifier::test {

int Func(int x, int y)
{
    return x + y;
}

struct Obj final {
    int operator()(int x, int y) const
    {
        return 2_I * x + y;
    }
    int SomeMethod(int x, int y) const
    {
        return x + y + 5_I;
    }
};

TEST_F(VerifierTest, callable)
{
    callable<int(int, int)> cal {};
    EXPECT_FALSE(cal);
    cal = Func;
    ASSERT_TRUE(cal);
    EXPECT_EQ(cal(3_I, 7_I), 10_I);
    Obj objExample;
    cal = objExample;
    ASSERT_TRUE(cal);
    EXPECT_EQ(cal(3_I, 7_I), 13_I);
    cal = callable<int(int, int)> {objExample, &Obj::SomeMethod};
    ASSERT_TRUE(cal);
    EXPECT_EQ(cal(3_I, 7_I), 15_I);
}

}  // namespace ark::verifier::test