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

class EtsOperatorNewEtsToJs : public EtsInteropTest {};

TEST_F(EtsOperatorNewEtsToJs, operator_new_no_arg)
{
    ASSERT_EQ(true, RunJsTestSuite("operator_new_no_arg.js"));
}
TEST_F(EtsOperatorNewEtsToJs, operator_new_one_arg)
{
    ASSERT_EQ(true, RunJsTestSuite("operator_new_one_arg.js"));
}
TEST_F(EtsOperatorNewEtsToJs, operator_new_two_arg)
{
    ASSERT_EQ(true, RunJsTestSuite("operator_new_two_arg.js"));
}
TEST_F(EtsOperatorNewEtsToJs, operator_new_many_arg)
{
    ASSERT_EQ(true, RunJsTestSuite("operator_new_many_arg.js"));
}
TEST_F(EtsOperatorNewEtsToJs, operator_new_sub_class)
{
    ASSERT_EQ(true, RunJsTestSuite("operator_new_sub_class.js"));
}

}  // namespace ark::ets::interop::js::testing
