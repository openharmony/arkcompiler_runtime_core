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

class EtsSetterEtsToTsTest : public EtsInteropTest {};

// NOTE (issues 17682) fix class setter
TEST_F(EtsSetterEtsToTsTest, DISABLED_check_setter_abstract)
{
    ASSERT_TRUE(RunJsTestSuite("check_setter_abstract.js"));
}
// NOTE (issues 17682) fix class setter
TEST_F(EtsSetterEtsToTsTest, DISABLED_check_setter_union)
{
    ASSERT_TRUE(RunJsTestSuite("check_setter_union.js"));
}
// NOTE (issues 17682) fix class setter
TEST_F(EtsSetterEtsToTsTest, DISABLED_check_setter_interface)
{
    ASSERT_TRUE(RunJsTestSuite("check_setter_interface.js"));
}
// NOTE (issues 17682) fix class setter
TEST_F(EtsSetterEtsToTsTest, DISABLED_check_setter_subset_ref)
{
    ASSERT_TRUE(RunJsTestSuite("check_setter_subset_ref.js"));
}
// NOTE (issues 17682) fix class setter
TEST_F(EtsSetterEtsToTsTest, DISABLED_check_setter_user_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_setter_user_class.js"));
}
// NOTE (issues 17682) fix class setter
TEST_F(EtsSetterEtsToTsTest, DISABLED_check_setter_subset_value)
{
    ASSERT_TRUE(RunJsTestSuite("check_setter_subset_value.js"));
}
// NOTE (issues 17682) fix class setter
TEST_F(EtsSetterEtsToTsTest, DISABLED_check_setter_any_type)
{
    ASSERT_TRUE(RunJsTestSuite("check_setter_any_type.js"));
}
// NOTE (issues 17682) fix class setter
TEST_F(EtsSetterEtsToTsTest, DISABLED_check_setter_tuple)
{
    ASSERT_TRUE(RunJsTestSuite("check_setter_tuple.js"));
}

}  // namespace ark::ets::interop::js::testing
