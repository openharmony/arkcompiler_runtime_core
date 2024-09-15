/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "ets_interop_js_gtest.h"

namespace ark::ets::interop::js::testing {

class EtsCallMethodForValueTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsCallMethodForValueTsToEtsTest, checkCallMethodForValue)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCallMethodForValue"));
}

TEST_F(EtsCallMethodForValueTsToEtsTest, checkCallMethodForValueWithOptionalOperator)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCallMethodForValueWithOptionalOperator"));
}
// NOTE (alexanderpolenov) issue(18449) enable this after fix non null operator
TEST_F(EtsCallMethodForValueTsToEtsTest, DISABLED_checkCallMethodForValueWithNotNullishOperator)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCallMethodForValueWithNotNullishOperator"));
}

}  // namespace ark::ets::interop::js::testing
