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

class EtsSetterTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsSetterTsToEtsTest, check_abstract_сlass_setter)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAbstractClassSetter"));
}

TEST_F(EtsSetterTsToEtsTest, check_union_setter)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkUnionSetter"));
}

TEST_F(EtsSetterTsToEtsTest, check_interface_setter)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInterfaceSetter"));
}

TEST_F(EtsSetterTsToEtsTest, check_user_class_setter)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkUserClassSetter"));
}

TEST_F(EtsSetterTsToEtsTest, check_extends_class)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkExtendsClass"));
}

TEST_F(EtsSetterTsToEtsTest, check_extends_class_with_value)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkExtendsClassWithValue"));
}

TEST_F(EtsSetterTsToEtsTest, check_tuple_type_object_form_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkTupleTypeObjectFormTs"));
}

TEST_F(EtsSetterTsToEtsTest, check_setter_any_type_object_form_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkSetterAnyTypeObjectFormTs"));
}

TEST_F(EtsSetterTsToEtsTest, check_setter_union_type_object_form_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkSetterUnionTypeObjectFormTs"));
}

TEST_F(EtsSetterTsToEtsTest, check_setter_interface_object_form_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkSetterInterfaceObjectFormTs"));
}

TEST_F(EtsSetterTsToEtsTest, check_setter_user_object_form_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkSetterUserObjectFormTs"));
}

TEST_F(EtsSetterTsToEtsTest, check_setter_subset_ref_set_object_form_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkSetterSubsetRefSetObjectFormTs"));
}

TEST_F(EtsSetterTsToEtsTest, check_setter_subset_value_set_object_form_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkSetterSubsetValueSetObjectFormTs"));
}

TEST_F(EtsSetterTsToEtsTest, check_setter_abstract_class_object_form_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkSetterAbstractClassObjectFormTs"));
}

}  // namespace ark::ets::interop::js::testing
