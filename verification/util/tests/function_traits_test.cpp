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

#include "util/function_traits.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

namespace panda::verifier::test {

struct square_sum {
    int operator()(int x, int y) const
    {
        return (x + y) * (x + y);
    }
};

struct square_diversity {
    int operator()(int x, int y) const
    {
        return (x - y) * (x - y);
    }
};

struct mult_by_mod {
    int mod;
    explicit mult_by_mod(int module) : mod {module} {}
    int operator()(int x, int y) const
    {
        return (x * y) % mod;
    }
};

TEST_F(VerifierTest, function_traits)
{
    square_sum sq_sum;
    square_diversity sq_div;
    n_ary<square_sum> op_s_sum {sq_sum};
    n_ary<square_diversity> op_s_div {sq_div};
    EXPECT_EQ(op_s_sum(2, 2), 16);
    EXPECT_EQ(op_s_div(2, 1), 1);
    EXPECT_EQ(op_s_sum(2, 1, 2), 121);
    EXPECT_EQ(op_s_div(2, 1, 2), 1);

    mult_by_mod mod5 {5};
    mult_by_mod mod10 {10};
    n_ary<mult_by_mod> op_mult_mod5 {mod5};
    n_ary<mult_by_mod> op_mult_mod10 {mod10};
    EXPECT_EQ(op_mult_mod5(2, 4), 3);
    EXPECT_EQ(op_mult_mod10(2, 4), 8);
    EXPECT_EQ(op_mult_mod5(2, 4, 2), 1);
    EXPECT_EQ(op_mult_mod10(2, 4, 2), 6);
}

}  // namespace panda::verifier::test