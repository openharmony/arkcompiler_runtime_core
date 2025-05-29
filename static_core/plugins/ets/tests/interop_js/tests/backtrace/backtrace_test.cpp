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

#include "ets_interop_js_gtest.h"

namespace ark::ets::interop::js::testing {

class BackTraceTest : public EtsInteropTest {};

TEST_F(BackTraceTest, simple_call)
{
    auto success = CallEtsFunction<int>(GetPackageName(), "TestBacktrace_entry_simple");
    ASSERT_EQ(success, 1);
}

TEST_F(BackTraceTest, complex_call)
{
    auto success = CallEtsFunction<int>(GetPackageName(), "TestBacktrace_entry_complex");
    ASSERT_EQ(success, 1);
}

}  // namespace ark::ets::interop::js::testing
