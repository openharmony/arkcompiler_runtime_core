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

#include <vector>
#include <array>
#include <map>

#include "util/struct_field.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

namespace panda::verifier::test {

struct MyArr final {
    int32_t num0 = 0;
    int32_t num1 = 1;
    int32_t num2 = 2;
    int32_t num3 = 3;
    int32_t num4 = 4;
    int32_t num5 = 5;

    using offset = struct_field<int32_t, int32_t>;

    std::map<char, offset> elem_map = {{'0', offset {0}},  {'1', offset {4}},  {'2', offset {8}},
                                       {'3', offset {12}}, {'4', offset {16}}, {'5', offset {20}}};

    int32_t &access(char id)
    {
        auto it = elem_map.find(id);
        ASSERT(it != elem_map.end());
        return it->second.of(num0);
    }
};

TEST_F(VerifierTest, struct_field)
{
    std::vector<int32_t> vec(5);
    vec[3] = 5;
    int32_t &pos1 = vec[1];
    struct_field<int32_t, int32_t> s_f1 {8};
    int32_t &pos2 = s_f1.of(pos1);
    EXPECT_EQ(pos2, 5);

    std::array<int64_t, 3> arr;
    arr[2] = 5;
    int64_t &pos3 = arr[0];
    struct_field<int64_t, int64_t> s_f2 {16};
    int64_t &pos4 = s_f2.of(pos3);
    EXPECT_EQ(pos4, 5);

    MyArr my_arr;
    EXPECT_EQ(my_arr.access('3'), 3);
    my_arr.access('4') = 44;
    EXPECT_EQ(my_arr.num4, 44);
}

}  // namespace panda::verifier::test