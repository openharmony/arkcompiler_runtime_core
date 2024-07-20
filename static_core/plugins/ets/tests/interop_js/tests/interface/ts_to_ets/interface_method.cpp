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

class EtsInterfaceTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsInterfaceTsToEtsTest, check_any_type_interface_class_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_any_type_interface_class_string"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_any_type_interface_class_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_any_type_interface_class_int"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_any_type_interface_class_bool)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_any_type_interface_class_bool"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_any_type_interface_class_array)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_any_type_interface_class_array"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_any_type_interface_class_object)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_any_type_interface_class_object"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_interface_class_any_type_method_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_interface_class_any_type_method_string"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_interface_class_any_type_method_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_interface_class_any_type_method_int"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_interface_class_any_type_method_bool)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_interface_class_any_type_method_bool"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_interface_class_any_type_method_array)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_interface_class_any_type_method_array"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_interface_class_any_type_method_object)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_interface_class_any_type_method_object"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_union_type_interface_class_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_union_type_interface_class_string"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_union_type_interface_class_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_union_type_interface_class_int"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_interface_class_union_type_method_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_interface_class_union_type_method_string"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_interface_class_union_type_method_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_interface_class_union_type_method_int"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_subset_by_ref_interface)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_subset_by_ref_interface"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_subset_by_ref_interface_error)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_subset_by_ref_interface_error"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_subset_by_value_class)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_subset_by_value_class"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_subset_by_value_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_subset_by_value_class_from_ts"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_instance_interface_class_union_type_method_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_instance_interface_class_union_type_method_int"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_instance_interface_class_union_type_method_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_instance_interface_class_union_type_method_string"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_instance_subset_by_value_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_instance_subset_by_value_class_from_ts"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_tuple_type_class)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_tuple_type_class"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_tuple_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_tuple_class_from_ts"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_instance_tuple_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_instance_tuple_class_from_ts"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_with_optional_method_class)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_with_optional_method_class"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_without_optional_method_class)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_without_optional_method_class"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_class_with_optional_method)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_class_with_optional_method"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_class_without_optional_method)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_class_without_optional_method"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_with_optional_method_instance_class)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_with_optional_method_instance_class"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_without_optional_method_instance_class)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_without_optional_method_instance_class"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_optional_arg_with_all_args)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_optional_arg_with_all_args"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_optional_arg_with_one_args)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_optional_arg_with_one_args"));
}
// NOTE (issues 17772) fix spread operator
TEST_F(EtsInterfaceTsToEtsTest, DISABLED_check_spread_operator_arg_with_all_args)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_spread_operator_arg_with_all_args"));
}
// NOTE (issues 17772) fix spread operator
TEST_F(EtsInterfaceTsToEtsTest, DISABLED_check_spread_operator_arg_with_one_args)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_spread_operator_arg_with_one_args"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_without_spread_operator_arg_with_all_args)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_without_spread_operator_arg_with_all_args"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_without_spread_operator_arg_with_one_args)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_without_spread_operator_arg_with_one_args"));
}

}  // namespace ark::ets::interop::js::testing
