/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class BackTraceTest : public AniTest {};

TEST_F(BackTraceTest, simple_call)
{
    ani_boolean unhandledError;
    // Call native library constructor
    auto funtRef = CallEtsFunction<ani_ref>("backtrace_test", "new_TestBacktrace");
    ASSERT_EQ(env_->ExistUnhandledError(&unhandledError), ANI_OK);
    ASSERT_EQ(unhandledError, ANI_FALSE) << "Cannot load native library";
    // Call native method
    auto success =
        CallEtsFunction<ani_int>("backtrace_test", "TestBacktrace_entry_simple", static_cast<ani_object>(funtRef));
    ASSERT_EQ(env_->ExistUnhandledError(&unhandledError), ANI_OK);
    ASSERT_EQ(unhandledError, ANI_FALSE) << "Cannot call native method";
    // Check result
    ASSERT_EQ(success, 1);
}

TEST_F(BackTraceTest, complex_call)
{
    ani_boolean unhandledError;
    // Call native library constructor
    auto funtRef = CallEtsFunction<ani_ref>("backtrace_test", "new_TestBacktrace");
    ASSERT_EQ(env_->ExistUnhandledError(&unhandledError), ANI_OK);
    ASSERT_EQ(unhandledError, ANI_FALSE) << "Cannot load native library";
    // Call native method
    auto success =
        CallEtsFunction<ani_int>("backtrace_test", "TestBacktrace_entry_complex", static_cast<ani_object>(funtRef));
    ASSERT_EQ(env_->ExistUnhandledError(&unhandledError), ANI_OK);
    ASSERT_EQ(unhandledError, ANI_FALSE) << "Cannot call native method";
    // Check result
    ASSERT_EQ(success, 1);
}

}  // namespace ark::ets::ani::testing
