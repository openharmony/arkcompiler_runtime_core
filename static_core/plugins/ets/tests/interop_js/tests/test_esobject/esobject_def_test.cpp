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

class EtsESObjectJsToEtsTest : public EtsInteropTest {};

TEST_F(EtsESObjectJsToEtsTest, test_esobject_get_undefined)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGetUndefined"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_get_null)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGetNull"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_wrap_boolean)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkWrapBoolean"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_wrap_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkWrapString"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_wrap_number)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkWrapNumber"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_wrap_byte)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkWrapByte"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_wrap_bigint)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkWrapBigInt"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_wrap_short)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkWrapShort"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_wrap_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkWrapInt"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_wrap_long)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkWrapLong"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_wrap_float)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkWrapFloat"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_wrap_Double)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkWrapDouble"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_is_boolean)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkIsBoolean"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_is_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkIsString"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_is_number)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkIsNumber"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_is_bigint)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkIsBigInt"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_is_undefined)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkIsUndefined"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_is_Function)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkIsFunction"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_to_boolean)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkToBoolean"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_to_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkToString"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_to_Number)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkToNumber"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_to_bigint)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkToBigInt"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_to_undefined)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkToUndefined"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_to_null)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkToNull"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_is_equal_to)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkIsEqualTo"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_are_strict_equal)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAreStrictEqual"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_is_equal_to_safe)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkIsEqualToSafe"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_get_property_by_name)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGetPropertyByName"));
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGetPropertyStaticObj"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_get_property_by_name_safe)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGetPropertyByNameSafe"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_get_property_by_index)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGetPropertyByIndex"));
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGetPropertyByIndexDouble"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_get_property_by_index_safe)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGetPropertyByIndexSafe"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_get_property)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGetProperty"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_get_property_safe)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGetPropertySafe"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_set_property_by_name)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkSetPropertyByName"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_set_property_by_index)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkSetPropertyByIndex"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_set_property)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkSetProperty"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_check_has_property)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkHasProperty"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_check_has_property_by_name)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkHasPropertyByName"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_check_type_of)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkTypeOf"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_check_invoke)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInvokeNoParam"));
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInvokeMethod"));
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInvokeHasParam"));
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInvokeMethodHasParam"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_check_iterator)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkIterator"));
    ASSERT_EQ(true, CallEtsMethod<bool>("checkKeys"));
    ASSERT_EQ(true, CallEtsMethod<bool>("checkValues"));
    ASSERT_EQ(true, CallEtsMethod<bool>("checkEntries"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_check_instanceOf)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceOfStaticObj"));
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceOfNumeric"));
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceOfStaticPrimitive"));
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceOfDynamic"));
}

TEST_F(EtsESObjectJsToEtsTest, test_esobject_check_instaniate)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstaniate"));
}

}  // namespace ark::ets::interop::js::testing