/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#include "nark_gtest.h"

namespace ark::ets::nark::testing {

class ExampleTest : public NarkTest {};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(ExampleTest, EtsFunctionCall)
{
    ets_double p1 = 5.0;
    ets_double p2 = 6.0;

    auto res = CallEtsFunction<ets_double>("exampleFunction", p1, p2);
    ASSERT_EQ(res, p1 + p2);
}

ets_long NativeFuncExample([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass, ets_long param1,
                           ets_long param2)
{
    return param1 * param2;
}

TEST_F(ExampleTest, CallNativeFunction)
{
    NativeFunction fn("nativeExampleFunction", NativeFuncExample);

    ets_long p1 = 12;
    ets_long p2 = -123;

    // Generic call
    auto res = CallEtsNativeMethod<ets_long>(fn, p1, p2);
    ASSERT_EQ(res, p1 * p2);
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::ets::nark::testing
