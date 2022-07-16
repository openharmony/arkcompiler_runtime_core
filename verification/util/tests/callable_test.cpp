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

#include "util/callable.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

namespace panda::verifier::test {

int func(int x, int y)
{
    return x + y;
}

struct obj final {
    int operator()(int x, int y) const
    {
        return 2 * x + y;
    }
    int some_method(int x, int y) const
    {
        return x + y + 5;
    }
};

TEST_F(VerifierTest, callable)
{
    callable<int(int, int)> cal {};
    EXPECT_FALSE(cal);
    cal = func;
    ASSERT_TRUE(cal);
    EXPECT_EQ(cal(3, 7), 10);
    obj obj_example;
    cal = obj_example;
    ASSERT_TRUE(cal);
    EXPECT_EQ(cal(3, 7), 13);
    cal = callable<int(int, int)> {obj_example, &obj::some_method};
    ASSERT_TRUE(cal);
    EXPECT_EQ(cal(3, 7), 15);
}

}  // namespace panda::verifier::test