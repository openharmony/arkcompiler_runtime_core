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

class EtsGetterTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsGetterTsToEtsTest, check_getter_public_class)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_getter_public_class"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_public_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_public_getter_class_from_ts"));
}

TEST_F(EtsGetterTsToEtsTest, check_public_getter_instance_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_public_getter_instance_class_from_ts"));
}

TEST_F(EtsGetterTsToEtsTest, check_union_type_getter_class_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_union_type_getter_class_int"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_union_type_getter_class_from_ts_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_union_type_getter_class_from_ts_int"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_union_type_getter_class_from_ts_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_instance_union_type_getter_class_from_ts_int"));
}

TEST_F(EtsGetterTsToEtsTest, check_union_type_getter_class_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_union_type_getter_class_string"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_union_type_getter_class_from_ts_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_union_type_getter_class_from_ts_string"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_union_type_getter_class_from_ts_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_instance_union_type_getter_class_from_ts_string"));
}

TEST_F(EtsGetterTsToEtsTest, check_literal_type_getter_class_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_literal_type_getter_class_int"));
}

TEST_F(EtsGetterTsToEtsTest, check_literal_type_getter_class_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_literal_type_getter_class_string"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_literal_type_getter_class_from_ts_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_literal_type_getter_class_from_ts_int"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_literal_type_getter_class_from_ts_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_literal_type_getter_class_from_ts_string"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_literal_type_getter_class_from_ts_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_instance_literal_type_getter_class_from_ts_int"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_literal_type_getter_class_from_ts_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_instance_literal_type_getter_class_from_ts_string"));
}

TEST_F(EtsGetterTsToEtsTest, check_tuple_type_getter_class)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_tuple_type_getter_class"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_tuple_type_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_tuple_type_getter_class_from_ts"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_tuple_type_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_instance_tuple_type_getter_class_from_ts"));
}

TEST_F(EtsGetterTsToEtsTest, check_any_type_getter_class_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_any_type_getter_class_int"));
}

TEST_F(EtsGetterTsToEtsTest, check_any_type_getter_class_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_any_type_getter_class_string"));
}

TEST_F(EtsGetterTsToEtsTest, check_any_type_getter_class_bool)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_any_type_getter_class_bool"));
}

TEST_F(EtsGetterTsToEtsTest, check_any_type_getter_class_arr)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_any_type_getter_class_arr"));
}

TEST_F(EtsGetterTsToEtsTest, check_any_type_getter_class_obj)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_any_type_getter_class_obj"));
}

TEST_F(EtsGetterTsToEtsTest, DISABLED_check_any_type_getter_class_tuple)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_any_type_getter_class_tuple"));
}

TEST_F(EtsGetterTsToEtsTest, check_any_type_getter_class_union)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_any_type_getter_class_union"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_any_type_getter_class_from_ts_int"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_any_type_getter_class_from_ts_string"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_bool)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_any_type_getter_class_from_ts_bool"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_arr)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_any_type_getter_class_from_ts_arr"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_obj)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_any_type_getter_class_from_ts_obj"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_tuple)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_any_type_getter_class_from_ts_tuple"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_union)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_any_type_getter_class_from_ts_union"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_instance_any_type_getter_class_from_ts_int"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_instance_any_type_getter_class_from_ts_string"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_bool)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_instance_any_type_getter_class_from_ts_bool"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_arr)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_instance_any_type_getter_class_from_ts_arr"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_obj)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_instance_any_type_getter_class_from_ts_obj"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_tuple)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_instance_any_type_getter_class_from_ts_tuple"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_union)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_instance_any_type_getter_class_from_ts_union"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_explicit_type_getter_class_from_ts_explicit)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_instance_any_explicit_type_getter_class_from_ts_explicit"));
}

TEST_F(EtsGetterTsToEtsTest, check_getter_subset_by_ref_class)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_getter_subset_by_ref_class"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_subset_by_ref_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_subset_by_ref_getter_class_from_ts"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_subset_by_ref_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_instance_subset_by_ref_getter_class_from_ts"));
}

TEST_F(EtsGetterTsToEtsTest, check_getter_subset_by_value_class)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_getter_subset_by_value_class"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_subset_by_value_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_subset_by_value_getter_class_from_ts"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_subset_by_value_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_instance_subset_by_value_getter_class_from_ts"));
}
}  // namespace ark::ets::interop::js::testing
