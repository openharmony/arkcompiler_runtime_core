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

#include "util/optional_ref.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

namespace panda::verifier::test {

TEST_F(VerifierTest, invalid_ref)
{
    int a = 2;
    OptionalRef<int> ref1 {a};
    OptionalConstRef<int> ref2 {a};
    OptionalRef<int> inv_ref1;
    OptionalRef<int> inv_ref12 = {};

    ASSERT_TRUE(ref1.HasRef());
    ASSERT_TRUE(ref2.HasRef());
    EXPECT_EQ(ref1.Get(), 2);
    EXPECT_EQ(ref2.Get(), 2);
    EXPECT_TRUE(!inv_ref1.HasRef());
    EXPECT_TRUE(!inv_ref12.HasRef());
}

}  // namespace panda::verifier::test