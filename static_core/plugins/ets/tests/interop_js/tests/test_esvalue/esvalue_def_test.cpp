/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

class EtsESValueJsToEtsTest : public EtsInteropTest {};

TEST_F(EtsESValueJsToEtsTest, test_esvalue_get_undefined)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetUndefined"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_get_null)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetNull"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_wrap_boolean)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkWrapBoolean"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_wrap_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkWrapString"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_wrap_number)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkWrapNumber"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_wrap_bigint)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkWrapBigInt"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_wrap_Double)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkWrapDouble"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_is_boolean)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsBoolean"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_is_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsString"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_is_number)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsNumber"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_is_bigint)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsBigInt"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_is_undefined)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsUndefined"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_is_Function)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsFunction"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_to_boolean)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkToBoolean"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_to_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkToString"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_to_Number)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkToNumber"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_to_bigint)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkToBigInt"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_to_undefined)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkToUndefined"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_to_null)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkToNull"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_is_equal_to)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsEqualTo"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_are_strict_equal)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAreStrictEqual"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_is_equal_to_safe)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsEqualToSafe"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_get_property_by_name)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetPropertyByName"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetPropertyStaticObj"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_get_property_by_name_safe)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetPropertyByNameSafe"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_get_property_by_index)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetPropertyByIndex"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetPropertyByIndexDouble"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_get_property_by_index_safe)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetPropertyByIndexSafe"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_get_property)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetProperty"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_get_property_safe)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetPropertySafe"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_set_property_by_name)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSetPropertyByName"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_set_property_by_index)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSetPropertyByIndex"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSetPropertyByIndexDouble"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_set_property)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSetProperty"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_check_has_property)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkHasProperty"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_check_has_property_by_name)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkHasPropertyByName"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_check_type_of)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTypeOf"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_check_invoke)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInvokeNoParam"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInvokeMethod"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInvokeHasParam"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInvokeMethodHasParam"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_check_iterator)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIterator"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkKeys"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkValues"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkEntries"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_check_instanceOf)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceOfStaticObj"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceOfNumeric"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceOfStaticPrimitive"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceOfDynamic"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceOfNullOrUndefined"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_check_instaniate)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstaniate"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_test_undefined)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testUndefined"));
}

TEST_F(EtsESValueJsToEtsTest, checkThrowNullOrUndefined)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkThrowNullOrUndefined"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_test_wrap_boundary)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapNaN"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapInfinity"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapNegativeInfinity"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapMaxSafeInteger"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapMinSafeInteger"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapMaxValue"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapMinValue"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapZero"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapEmptyString"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapMaxBigInt"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapByteMin"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapByteMax"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapShortMin"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapShortMax"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapIntMin"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapIntMax"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapLargeLong"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapFloatMax"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapNull"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapStaticObject"));
}

TEST_F(EtsESValueJsToEtsTest, typeConversionExceptionTests)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testToBooleanThrowsOnNumber"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testToStringThrowsOnNumber"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testToNumberThrowsOnString"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testToBigIntThrowsOnNumber"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testToUndefinedThrowsOnNull"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testToNullThrowsOnUndefined"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testToStaticObjectThrowsOnECMAObject"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapUndefined"));
}

TEST_F(EtsESValueJsToEtsTest, testInvokeMethodThrowsOnNonExistent)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInvokeMethodThrowsOnNonExistent"));
}

TEST_F(EtsESValueJsToEtsTest, testIteratorThrowsOnNonIterable)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testIteratorThrowsOnNonIterable"));
}

TEST_F(EtsESValueJsToEtsTest, testToPromiseThrowsOnNonPromise)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testToPromiseThrowsOnNonPromise"));
}

TEST_F(EtsESValueJsToEtsTest, testUnwrap)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testUnwrap"));
}

TEST_F(EtsESValueJsToEtsTest, testInstantiateEmptyObject)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInstantiateEmptyObject"));
}

TEST_F(EtsESValueJsToEtsTest, testInstantiateEmptyArray)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInstantiateEmptyArray"));
}

TEST_F(EtsESValueJsToEtsTest, testGetGlobal)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testGetGlobal"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testGetGlobalProperty"));
}

TEST_F(EtsESValueJsToEtsTest, testIsECMAObject)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testIsECMAObject"));
}

TEST_F(EtsESValueJsToEtsTest, testIsObjectWithStatic)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testIsObjectWithStatic"));
}

TEST_F(EtsESValueJsToEtsTest, testIsObjectWithECMA)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testIsObjectWithECMA"));
}

TEST_F(EtsESValueJsToEtsTest, testAreEqualSafe)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testAreEqualSafeNullNull"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testAreEqualSafeUndefUndef"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testAreEqualSafeNullUndef"));
}

TEST_F(EtsESValueJsToEtsTest, testIsEqualToSafe)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testIsEqualToSafe"));
}

TEST_F(EtsESValueJsToEtsTest, testIsPromise)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testIsPromise"));
}

TEST_F(EtsESValueJsToEtsTest, testIsNotPromise)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testIsNotPromise"));
}

TEST_F(EtsESValueJsToEtsTest, testIndexAccess)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testNegativeIndexAccess"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testLargeIndexAccess"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testNaNIndexAccess"));
}

TEST_F(EtsESValueJsToEtsTest, testHasOwnProperty)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testHasOwnProperty"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testHasOwnPropertyWithESValue"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testHasPropertyVsHasOwnProperty"));
}

TEST_F(EtsESValueJsToEtsTest, testInvokeWithRecv)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInvokeWithRecv"));
}

TEST_F(EtsESValueJsToEtsTest, testGetPropertySafe)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testGetPropertySafeNumberNonExistent"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testGetPropertySafeESValueNonExistent"));
}

TEST_F(EtsESValueJsToEtsTest, testSetProperty)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSetPropertyToUndefined"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSetPropertyToNull"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSetPropertyWithESValue"));
}

TEST_F(EtsESValueJsToEtsTest, testWrapLongLossy)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWrapLongLossy"));
}

TEST_F(EtsESValueJsToEtsTest, testEmptyObjectIteration)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testEmptyObjectIteration"));
}

TEST_F(EtsESValueJsToEtsTest, testEmptyArrayIteration)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testEmptyArrayIteration"));
}

TEST_F(EtsESValueJsToEtsTest, testComplexObjectNestedAccess)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testComplexObjectNestedAccess"));
}

TEST_F(EtsESValueJsToEtsTest, testObjectWithMethodsInvocation)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testObjectWithMethodsInvocation"));
}

TEST_F(EtsESValueJsToEtsTest, testObjWithNullsAccess)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testObjWithNullsAccess"));
}

TEST_F(EtsESValueJsToEtsTest, testJSClassInstantiation)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testJSClassInstantiation"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testJSClassInstanceProperties"));
}

TEST_F(EtsESValueJsToEtsTest, testCustomIterable)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCustomIterableManualNext"));
}

TEST_F(EtsESValueJsToEtsTest, testHasOwnPropertyVsHasPropertyWithProto)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testHasOwnPropertyVsHasPropertyWithProto"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testHasOwnPropertyVsHasPropertyWithInheritance"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testHasOwnPropertyOnClassConstructor"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testHasOwnPropertyOnMethods"));
}

TEST_F(EtsESValueJsToEtsTest, testComplexObjectPropertyModification)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testComplexObjectPropertyModification"));
}

TEST_F(EtsESValueJsToEtsTest, testClassMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testClassMethodOverride"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testClassUniqueMethod"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testClassInheritedMethod"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testClassSuperMethodCall"));
}

TEST_F(EtsESValueJsToEtsTest, testClassInstanceOfInheritance)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testClassInstanceOfInheritance"));
}

TEST_F(EtsESValueJsToEtsTest, testClassModifyInheritedFields)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testClassModifyInheritedFields"));
}

TEST_F(EtsESValueJsToEtsTest, testSymbolPropertyAccess)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSymbolPropertyAccess"));
}

TEST_F(EtsESValueJsToEtsTest, testESValeueReflect)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testReflectHasProperty"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testReflectOwnKeys"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testReflectGetOwnPropertyDescriptor"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testReflectApply"));
}

TEST_F(EtsESValueJsToEtsTest, testObjectImmutability)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testObjectIsFrozenAndSealed"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testObjectIsExtensible"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testFrozenObjectCannotModify"));
}

TEST_F(EtsESValueJsToEtsTest, testFunctionOperations)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testFunctionBind"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testFunctionApply"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testFunctionCall"));
}

TEST_F(EtsESValueJsToEtsTest, testESValueOptJson)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testJsonStringify"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testJsonParse"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testJsonSpecialValues"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testJsonCircularReference"));
}

TEST_F(EtsESValueJsToEtsTest, testMapOperations)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testMapOperations"));
}

TEST_F(EtsESValueJsToEtsTest, testSetOperations)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSetOperations"));
}

TEST_F(EtsESValueJsToEtsTest, testWeakMapOperations)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWeakMapOperations"));
}

TEST_F(EtsESValueJsToEtsTest, testWeakSetOperations)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWeakSetOperations"));
}

TEST_F(EtsESValueJsToEtsTest, testSparseArray)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSparseArray"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testArrayWithHoles"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testArrayLikeObject"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testNonElementProperties"));
}

TEST_F(EtsESValueJsToEtsTest, testDateOperations)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testDateCreation"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testDateGetTime"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInvalidDate"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testDateToISOString"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testDateGetMethods"));
}
}  // namespace ark::ets::interop::js::testing