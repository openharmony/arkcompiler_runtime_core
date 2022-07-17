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

#include "util/enum_array.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

namespace {

enum class Numbers { ONE, TWO, THREE, FOUR, FIVE };

}  // namespace

namespace panda::verifier::test {

TEST_F(VerifierTest, EnumArray)
{
    EnumArray<int, Numbers, Numbers::ONE, Numbers::TWO, Numbers::THREE, Numbers::FOUR, Numbers::FIVE> en_arr {};

    en_arr[Numbers::TWO] = 9;
    en_arr[Numbers::FIVE] = 5;

    EXPECT_EQ(en_arr[Numbers::ONE], 0);
    EXPECT_EQ(en_arr[Numbers::TWO], 9);
    EXPECT_EQ(en_arr[Numbers::FIVE], 5);
}

}  // namespace panda::verifier::test