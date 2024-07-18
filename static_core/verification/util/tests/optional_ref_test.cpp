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

#include "libpandabase/utils/utils.h"
#include "util/optional_ref.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

namespace ark::verifier::test {

TEST_F(VerifierTest, invalid_ref)
{
    int a = 2;
    OptionalRef<int> ref1 {a};
    OptionalConstRef<int> ref2 {a};
    OptionalRef<int> invRef1;
    OptionalRef<int> invRef12 = {};

    ASSERT_TRUE(ref1.HasRef());
    ASSERT_TRUE(ref2.HasRef());
    EXPECT_EQ(ref1.Get(), 2_I);
    EXPECT_EQ(ref2.Get(), 2_I);
    EXPECT_TRUE(!invRef1.HasRef());
    EXPECT_TRUE(!invRef12.HasRef());
}

}  // namespace ark::verifier::test
