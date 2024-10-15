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

class EtsInteropScenariosJsToEtsIndirectCallValue : public EtsInteropTest {};

TEST_F(EtsInteropScenariosJsToEtsIndirectCallValue, Test_indirect_call_type_value_number_call)
{
    auto ret = CallEtsMethod<bool>("Test_indirect_call_type_value_number_call");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEtsIndirectCallValue, Test_indirect_call_type_value_number_apply)
{
    auto ret = CallEtsMethod<bool>("Test_indirect_call_type_value_number_apply");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEtsIndirectCallValue, Test_indirect_call_type_value_number_bind_with_arg)
{
    auto ret = CallEtsMethod<bool>("Test_indirect_call_type_value_number_bind_with_arg");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEtsIndirectCallValue, Test_indirect_call_type_value_number_bind_without_arg)
{
    auto ret = CallEtsMethod<bool>("Test_indirect_call_type_value_number_bind_without_arg");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEtsIndirectCallValue, Test_indirect_call_type_value_string_call)
{
    auto ret = CallEtsMethod<bool>("Test_indirect_call_type_value_string_call");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEtsIndirectCallValue, Test_indirect_call_type_value_string_apply)
{
    auto ret = CallEtsMethod<bool>("Test_indirect_call_type_value_string_apply");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEtsIndirectCallValue, Test_indirect_call_type_value_string_bind_with_arg)
{
    auto ret = CallEtsMethod<bool>("Test_indirect_call_type_value_string_bind_with_arg");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEtsIndirectCallValue, Test_indirect_call_type_value_string_bind_without_arg)
{
    auto ret = CallEtsMethod<bool>("Test_indirect_call_type_value_string_bind_without_arg");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEtsIndirectCallValue, Test_indirect_call_type_value_boolean_call)
{
    auto ret = CallEtsMethod<bool>("Test_indirect_call_type_value_boolean_call");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEtsIndirectCallValue, Test_indirect_call_type_value_boolean_apply)
{
    auto ret = CallEtsMethod<bool>("Test_indirect_call_type_value_boolean_apply");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEtsIndirectCallValue, Test_indirect_call_type_value_boolean_bind_with_arg)
{
    auto ret = CallEtsMethod<bool>("Test_indirect_call_type_value_boolean_bind_with_arg");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEtsIndirectCallValue, Test_indirect_call_type_value_boolean_bind_without_arg)
{
    auto ret = CallEtsMethod<bool>("Test_indirect_call_type_value_boolean_bind_without_arg");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
