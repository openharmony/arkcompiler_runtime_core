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

#include <gtest/gtest.h>
#include "ets_interop_js_gtest.h"

namespace ark::ets::interop::js::testing {

class EtsEsObjectStringLiteralKeyTest : public EtsInteropTest {};

TEST_F(EtsEsObjectStringLiteralKeyTest, check_field_access_by_dot)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "check_field_access_by_dot");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsEsObjectStringLiteralKeyTest, check_esobject_with_string_literal)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "check_esobject_with_string_literal");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsEsObjectStringLiteralKeyTest, check_esobject_with_string_key)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "check_esobject_with_string_key");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsEsObjectStringLiteralKeyTest, check_esobject_with_string_from_foo)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "check_esobject_with_string_from_foo");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsEsObjectStringLiteralKeyTest, check_esobject_with_expression)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "check_esobject_with_expression");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsEsObjectStringLiteralKeyTest, check_esobject_with_expression_str_literals)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "check_esobject_with_expression");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsEsObjectStringLiteralKeyTest, check_esobject_with_expression_strings)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "check_esobject_with_expression");
    ASSERT_EQ(ret, 0);
}
}  // namespace ark::ets::interop::js::testing