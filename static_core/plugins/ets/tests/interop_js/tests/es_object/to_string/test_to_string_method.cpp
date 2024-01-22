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

class ToStringTest : public EtsInteropTest {};

TEST_F(ToStringTest, implicit_to_string)
{
    ASSERT_EQ(CallEtsMethod<std::string>("implicit_to_string"), "optional_valA");
}

TEST_F(ToStringTest, test_functional1)
{
    ASSERT_EQ(CallEtsMethod<std::string>("test_functional1"), "Str1A");
}

TEST_F(ToStringTest, test_functional2)
{
    ASSERT_EQ(CallEtsMethod<std::string>("test_functional2"), "123A");
}

TEST_F(ToStringTest, test_nan_call)
{
    ASSERT_EQ(CallEtsMethod<std::string>("test_nan_call"), "NaNA");
}

TEST_F(ToStringTest, test_throwing)
{
    ASSERT_EQ(CallEtsMethod<std::string>("test_throwing"), "123A");
}

TEST_F(ToStringTest, ets_jsvalue_to_str)
{
    ASSERT_EQ(CallEtsMethod<std::string>("ets_jsvalue_to_str"), "123AA");
}

TEST_F(ToStringTest, ets_object_to_str)
{
    ASSERT_EQ(CallEtsMethod<std::string>("ets_object_to_str"), "123AA");
}

TEST_F(ToStringTest, jsval_object_to_str)
{
    ASSERT_EQ(CallEtsMethod<std::string>("jsval_object_to_str"), "123A");
}

TEST_F(ToStringTest, concat_null_to_string)
{
    ASSERT_EQ(CallEtsMethod<std::string>("concat_null_to_string"), "nullA");
}

TEST_F(ToStringTest, concat_undefined_to_string)
{
    ASSERT_EQ(CallEtsMethod<std::string>("concat_undefined_to_string"), "undefinedA");
}

TEST_F(ToStringTest, concat_nan_to_string)
{
    ASSERT_EQ(CallEtsMethod<std::string>("concat_nan_to_string"), "NaNA");
}

TEST_F(ToStringTest, explicit_optional_to_string)
{
    ASSERT_EQ(CallEtsMethod<std::string>("explicit_optional_to_string"), "NPE");
}

}  // namespace ark::ets::interop::js::testing
