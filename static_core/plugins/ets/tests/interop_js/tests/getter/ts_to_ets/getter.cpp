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
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGetterPublicClass"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_public_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreatePublicGetterClassFromTs"));
}

TEST_F(EtsGetterTsToEtsTest, check_public_getter_instance_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkPublicGetterInstanceClassFromTs"));
}

TEST_F(EtsGetterTsToEtsTest, check_union_type_getter_class_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkUnionTypeGetterClassInt"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_union_type_getter_class_from_ts_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateUnionTypeGetterClassFromTsInt"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_union_type_getter_class_from_ts_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceUnionTypeGetterClassFromTsInt"));
}

TEST_F(EtsGetterTsToEtsTest, check_union_type_getter_class_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkUnionTypeGetterClassString"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_union_type_getter_class_from_ts_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateUnionTypeGetterClassFromTsString"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_union_type_getter_class_from_ts_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceUnionTypeGetterClassFromTsString"));
}

TEST_F(EtsGetterTsToEtsTest, check_literal_type_getter_class_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkLiteralTypeGetterClassInt"));
}

TEST_F(EtsGetterTsToEtsTest, check_literal_type_getter_class_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkLiteralTypeGetterClassString"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_literal_type_getter_class_from_ts_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateLiteralTypeGetterClassFromTsInt"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_literal_type_getter_class_from_ts_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateLiteralTypeGetterClassFromTsString"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_literal_type_getter_class_from_ts_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceLiteralTypeGetterClassFromTsInt"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_literal_type_getter_class_from_ts_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceLiteralTypeGetterClassFromTsString"));
}

TEST_F(EtsGetterTsToEtsTest, check_tuple_type_getter_class)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkTupleTypeGetterClass"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_tuple_type_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateTupleTypeGetterClassFromTs"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_tuple_type_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceTupleTypeGetterClassFromTs"));
}

TEST_F(EtsGetterTsToEtsTest, check_any_type_getter_class_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeGetterClassInt"));
}

TEST_F(EtsGetterTsToEtsTest, check_any_type_getter_class_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeGetterClassString"));
}

TEST_F(EtsGetterTsToEtsTest, check_any_type_getter_class_bool)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeGetterClassBool"));
}

TEST_F(EtsGetterTsToEtsTest, check_any_type_getter_class_arr)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeGetterClassArr"));
}

TEST_F(EtsGetterTsToEtsTest, check_any_type_getter_class_obj)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeGetterClassObj"));
}

TEST_F(EtsGetterTsToEtsTest, DISABLED_check_any_type_getter_class_tuple)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeGetterClassTuple"));
}

TEST_F(EtsGetterTsToEtsTest, check_any_type_getter_class_union)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeGetterClassUnion"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateAnyTypeGetterClassFromTsInt"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateAnyTypeGetterClassFromTsString"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_bool)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateAnyTypeGetterClassFromTsBool"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_arr)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateAnyTypeGetterClassFromTsArr"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_obj)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateAnyTypeGetterClassFromTsObj"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_tuple)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateAnyTypeGetterClassFromTsTuple"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_union)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateAnyTypeGetterClassFromTsUnion"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceAnyTypeGetterClassFromTsInt"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceAnyTypeGetterClassFromTsString"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_bool)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceAnyTypeGetterClassFromTsBool"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_arr)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceAnyTypeGetterClassFromTsArr"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_obj)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceAnyTypeGetterClassFromTsObj"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_tuple)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceAnyTypeGetterClassFromTsTuple"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_union)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceAnyTypeGetterClassFromTsUnion"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_explicit_type_getter_class_from_ts_explicit)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceAnyExplicitTypeGetterClassFromTsExplicit"));
}

TEST_F(EtsGetterTsToEtsTest, check_getter_subset_by_ref_class)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGetterSubsetByRefClass"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_subset_by_ref_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateSubsetByRefGetterClassFromTs"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_subset_by_ref_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceSubsetByRefGetterClassFromTs"));
}

TEST_F(EtsGetterTsToEtsTest, check_getter_subset_by_value_class)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGetterSubsetByValueClass"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_subset_by_value_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateSubsetByValueGetterClassFromTs"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_subset_by_value_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceSubsetByValueGetterClassFromTs"));
}
}  // namespace ark::ets::interop::js::testing
