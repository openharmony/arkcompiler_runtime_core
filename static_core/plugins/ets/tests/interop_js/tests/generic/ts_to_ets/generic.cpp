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

class EtsGenericTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsGenericTsToEtsTest, check_literal_class_generic)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkLiteralClassGeneric"));
}

TEST_F(EtsGenericTsToEtsTest, check_union_class_generic)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkUnionClassGeneric"));
}

TEST_F(EtsGenericTsToEtsTest, check_interface_class_generic)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInterfaceClassGeneric"));
}

TEST_F(EtsGenericTsToEtsTest, check_abstract_class_generic)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAbstractClassGeneric"));
}

TEST_F(EtsGenericTsToEtsTest, check_generic_class_generic)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericClassGeneric"));
}

TEST_F(EtsGenericTsToEtsTest, check_generic_literal_class_object_form_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericLiteralClassObjectFromTs"));
}

TEST_F(EtsGenericTsToEtsTest, check_generic_union_class_object_form_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericUnionClassObjectFromTs"));
}

TEST_F(EtsGenericTsToEtsTest, check_generic_interface_class_object_form_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericInterfaceClassObjectFromTs"));
}

TEST_F(EtsGenericTsToEtsTest, check_generic_abstract_class_object_form_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericAbstractClassObjectFromTs"));
}

TEST_F(EtsGenericTsToEtsTest, check_generic_function_type_any_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericFunctionTypeAnyString"));
}

TEST_F(EtsGenericTsToEtsTest, check_generic_function_type_any_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericFunctionTypeAnyInt"));
}

TEST_F(EtsGenericTsToEtsTest, check_generic_function_type_any_bool)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericFunctionTypeAnyBool"));
}

TEST_F(EtsGenericTsToEtsTest, check_generic_function_tuple_type)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericFunctionTupleType"));
}

TEST_F(EtsGenericTsToEtsTest, check_generic_explicitly_declared_type)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericExplicitlyDeclaredType"));
}

}  // namespace ark::ets::interop::js::testing
