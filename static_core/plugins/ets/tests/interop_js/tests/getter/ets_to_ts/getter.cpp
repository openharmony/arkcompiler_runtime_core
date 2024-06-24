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

class EtsGetterEtsToTsTest : public EtsInteropTest {};

// NOTE: issues (17842) getter undefined
TEST_F(EtsGetterEtsToTsTest, DISABLED_check_public_getter_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_public_getter_class.js"));
}
// NOTE: issues (17842) getter undefined
TEST_F(EtsGetterEtsToTsTest, DISABLED_check_protected_getter_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_protected_getter_class.js"));
}
// NOTE: issues (17842) getter undefined
TEST_F(EtsGetterEtsToTsTest, DISABLED_check_protected_getter_inheritance_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_protected_getter_inheritance_class.js"));
}
// NOTE: issues (17842) getter undefined
TEST_F(EtsGetterEtsToTsTest, DISABLED_check_private_getter_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_private_getter_class.js"));
}
// NOTE: issues (17842) getter undefined
TEST_F(EtsGetterEtsToTsTest, DISABLED_check_private_getter_inheritance_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_private_getter_inheritance_class.js"));
}
// NOTE: issues (17842) getter undefined
TEST_F(EtsGetterEtsToTsTest, DISABLED_check_union_type_getter_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_union_type_getter_class.js"));
}
// NOTE: issues (17842) getter undefined
TEST_F(EtsGetterEtsToTsTest, DISABLED_check_any_type_getter_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_any_type_getter_class.js"));
}
// NOTE: issues (17842) getter undefined
TEST_F(EtsGetterEtsToTsTest, DISABLED_check_subset_by_ref_getter_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_subset_by_ref_getter_class.js"));
}
// NOTE: issues (17842) getter undefined
TEST_F(EtsGetterEtsToTsTest, DISABLED_check_subset_by_value_getter_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_subset_by_value_getter_class.js"));
}

}  // namespace ark::ets::interop::js::testing
