/*
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

let module = ESValue.load('esvalue_def_test');
let module2 = ESValue.load('esvalue_def_test2');

let jsBoolean = module.getProperty('b');
let jsNumber = module.getProperty('n');
let jsString = module.getProperty('s');
let jsBoxNum = module2.getProperty('boxNum');
let jsBoxBool = module2.getProperty('boxBool');
let jsBoxStr = module2.getProperty('boxStr');
let jsUndefined = module.getProperty('jsUndefined');
let jsBigInt = module.getProperty('bi');
let jsNull = module.getProperty('jsNull');
let jsFunc = module.getProperty('jsFunc');
let jsFuncWithParam = module.getProperty('jsFuncWithParam');
let jsArray = module.getProperty('jsArray');
let jsArray1 = module.getProperty('jsArray1');
let jsObjectA = module.getProperty('A');
let jsObjectB = module.getProperty('B');
let jsObjWithFunc = module.getProperty('objWithFunc');
let jsObjWithFuncParam = module.getProperty('objWithFuncParam');
let jsIterable = module.getProperty('jsIterable');
let jsIterableObject = module.getProperty('testItetatorObject');
let jsUser = module.getProperty('User');
let jsDoubledObj = module.getProperty('doubledObj');
let propObj = module.getProperty('propObj');
let throwNull = module.getProperty('throwNull');
let throwUndefined = module.getProperty('throwUndefined');

function checkGetUndefined(): boolean {
    let obj = ESValue.Undefined;
    return obj.typeOf() === 'undefined';
}

function checkGetNull(): boolean {
    let obj = ESValue.Null;
    return obj.isNull();
}

function checkWrapBoolean(): boolean {
    let val = false;
    let obj = ESValue.wrapBoolean(val);
    return obj.toBoolean() === val;
}

function checkWrapString(): boolean {
    let val = 'hello';
    let obj = ESValue.wrapString(val);
    return obj.toString() === val;
}

function checkWrapBigInt(): boolean {
    let val = new BigInt(9007199254740991n);
    let obj = ESValue.wrapBigInt(val);
    return obj.toBigInt() === val;
}

function checkWrapNumber(): boolean {
    let val = 111;
    let obj = ESValue.wrapNumber(val);
    return obj.toNumber() === val;
}

function checkWrapDouble(): boolean {
    let val: double = 111;
    let obj = ESValue.wrapDouble(val);
    return obj.toNumber() === val;
}

function checkIsBoolean(): boolean {
    return jsBoolean.isBoolean();
}

function checkIsString(): boolean {
    return jsString.isString();
}

function checkIsNumber(): boolean {
    return jsNumber.isNumber();
}

function checkIsBigInt(): boolean {
    return jsBigInt.isBigInt();
}

function checkIsUndefined(): boolean {
    let obj = ESValue.Undefined;
    return obj.isUndefined();
}

function checkIsFunction(): boolean {
    return jsFunc.isFunction();
}

function checkToBoolean(): boolean {
    return jsBoolean.toBoolean() === false;
}

function checkToString(): boolean {
    return jsString.toString() === 'hello';
}

function checkToNumber(): boolean {
    return jsNumber.toNumber() === 1;
}

function checkToBigInt(): boolean {
    return jsBigInt.toBigInt() === 9007199254740991n;
}

function checkToUndefined(): boolean {
    return jsUndefined.toUndefined() === undefined;
}

function checkToNull(): boolean {
    return jsNull.toNull() === null;
}

function checkAreStrictEqual(): boolean {
    return ESValue.areStrictlyEqual(jsFunc, jsFunc);
}

function checkIsEqualTo(): boolean {
    return jsFunc.isEqualTo(jsFunc);
}

function checkIsEqualToSafe(): boolean {
    let nullObj1 = ESValue.Null;
    let nullObj2 = ESValue.Null;
    return nullObj1.isEqualToSafe(nullObj2);
}

function checkGetPropertyByName(): boolean {
    let val = jsObjectA.getProperty('property2');
    return val.toNumber() === 2;
}

class staticObjectA {
    aaa: string = ''
}
function checkGetPropertyStaticObj(): boolean {
    let sObj = new staticObjectA();
    sObj.aaa = 'bbb';
    let obj = ESValue.wrap(sObj);
    let value = obj.getPropertySafe('aaa');
    return value.toString() === 'bbb';
}

function checkGetPropertyByNameSafe(): boolean {
    let val = jsObjectA.getPropertySafe('property3');
    return val.isUndefined() === true;
}

function checkGetPropertyByIndex(): boolean {
    let val = jsArray.getProperty(2);
    return val.toBoolean() === true;
}

function checkGetPropertyByIndexDouble(): boolean {
    let index = 3.2;
    let val = jsArray.getPropertySafe(3.2);
    let val1 = jsDoubledObj.getProperty(index);
    return val1.toString() === 'aaa';
}

function checkGetPropertyByIndexSafe(): boolean {
    let val = jsArray.getPropertySafe(3);
    return val.isUndefined() === true;
}

function checkGetProperty(): boolean {
    let property = ESValue.wrapString('property2');
    let val = jsObjectA.getProperty(property);
    return val.toNumber() === 2;
}

function checkGetPropertySafe(): boolean {
    let peroperty = ESValue.wrapString('property3');
    let val = jsObjectA.getPropertySafe(peroperty);
    return val.isUndefined() === true;
}

function checkSetPropertyByName(): boolean {
    let value = ESValue.wrapNumber(5);
    jsObjectA.setProperty('property1', value);
    let val = jsObjectA.getProperty('property1');
    return val.toNumber() === 5;
}

function checkSetPropertyByIndex(): boolean {
    let value = ESValue.wrapBoolean(false);
    jsArray1.setProperty(2, value);
    let val = jsArray1.getProperty(2);
    return val.toBoolean() === false;
}

function checkSetPropertyByIndexDouble(): boolean {
    let index = 4.2;
    let val = 'cccc';
    jsDoubledObj.setProperty(index, val);
    jsDoubledObj.setProperty(ESValue.wrap(4.5), val);
    return jsDoubledObj.getProperty(index).toString() === val &&
        jsDoubledObj.getProperty(ESValue.wrap(4.5)).toString() === val;
}

function checkSetProperty(): boolean {
    let value = ESValue.wrapNumber(5);
    let property = ESValue.wrapString('property1');
    jsObjectA.setProperty(property, value);
    let val = jsObjectA.getProperty(property);
    return val.toNumber() === 5;
}

function checkHasProperty(): boolean {
    let property = ESValue.wrapString('property2');
    return jsObjectB.hasProperty(property) === true;
}

function checkHasPropertyByName(): boolean {
    return jsObjectB.hasProperty('property2');
}

function checkTypeOf(): boolean {
    let jsType = jsString.typeOf();
    let jsNumType = jsBoxNum.typeOf();
    let jsBoolType = jsBoxBool.typeOf();
    let jsStrType = jsBoxStr.typeOf();
    return jsType === 'string' &&
        jsNumType === 'number' &&
        jsBoolType === 'boolean' &&
        jsStrType === 'string';
}

function checkInvokeNoParam(): boolean {
    let result = jsFunc.invoke();
    if (!result.isNumber()) {
        return false;
    }
    return result.toNumber() === 6;
}

function checkInvokeHasParam(): boolean {
    let args: FixedArray<(Object | null | undefined)> = [ESValue.wrapDouble(2), ESValue.wrapDouble(3)];
    let result: ESValue = jsFuncWithParam.invoke(...args);
    if (!result.isNumber()) {
        return false;
    }
    return result.toNumber() === 5;
}

function checkInvokeMethod(): boolean {
    let result = jsObjWithFunc.invokeMethod('func');
    if (!result.isString()) {
        return false;
    }
    return result.toString() === 'hello';
}

function checkInvokeMethodHasParam(): boolean {
    let args: FixedArray<ESValue> = [ESValue.wrapString('abc'), ESValue.wrapString('def')];
    let result = jsObjWithFuncParam.invokeMethod('hello', ...args);
    if (!result.isString()) {
        return false;
    }
    return result.toString() === 'abcdef';
}

function checkIterator(): boolean {
    let result: String = new String();
    for (const elem of jsIterable) {
        result += elem.toString();
    }
    return result === 'abcd';
}

function checkKeys(): boolean {
    let keys = ['a', 'b', 'c'];
    let index = 0;
    for (const key of jsIterableObject.keys()) {
        if (key.toString() !== keys[index]) {
            return false;
        }
        index++;
    }
    if (index !== 3) {
        return false;
    }
    return true;
}

function checkValues(): boolean {
    let values = [1, 2, 3];
    let index = 0;
    for (const value of jsIterableObject.values()) {
        if (value.toNumber() !== values[index]) {
            return false;
        }
        index++;
    }
    if (index !== 3) {
        return false;
    }
    return true;
}

function checkEntries(): boolean {
    let keys = ['a', 'b', 'c'];
    let values = [1, 2, 3];
    let index = 0;
    for (const entry of jsIterableObject.entries()) {
        if (entry[0].toString() !== keys[index]) {
            return false;
        }
        if (entry[1].toNumber() !== values[index]) {
            return false;
        }
        index++;
    }
    if (index !== 3) {
        return false;
    }
    return true;
}

class testObj { }
function checkInstanceOfStaticObj(): boolean {
    let staticObj = new testObj();
    let obj = ESValue.wrap(staticObj);
    return obj.instanceOf(Type.of(staticObj));
}

function checkInstanceOfNumeric(): boolean {
    let num: short = 3;
    let obj = ESValue.wrapNumber(num);
    return obj.instanceOf(Type.of(num));
}

function checkInstanceOfStaticPrimitive(): boolean {
    let boolObj = false;
    let obj = ESValue.wrapBoolean(boolObj);
    return obj.instanceOf(Type.of(boolObj));
}

function checkInstanceOfDynamic(): boolean {
    let num = ESValue.wrapInt(123);
    let user = jsUser.instantiate(num);
    return user.instanceOf(jsUser);
}

function checkInstanceOfNullOrUndefined(): boolean {
    let undefValue = module2.getProperty('undefValue');
    let nullValue = module2.getProperty('nullValue');
    let res = true;
    res = res && !nullValue.instanceOf(nullValue);
    try {
        res = res && undefValue.instanceOf(nullValue);
        res = false;
    } catch (e) {
        res = res && e.message.includes('Need object');
    }
    try {
        res = res && nullValue.instanceOf(undefValue);
        res = false;
    } catch (e) {
        res = res && e.message.includes('Need object');
    }
    try {
        res = res && undefValue.instanceOf(undefValue);
        res = false;
    } catch (e) {
        res = res && e.message.includes('Need object');
    }
    return res;
}

function checkInstaniate(): boolean {
    let num = ESValue.wrapInt(123);
    let user = jsUser.instantiate(num);
    return user.getProperty('ID').toNumber() === 123;
}

function testUndefined(): boolean {
    propObj.setProperty('property1', undefined);
    propObj.setProperty('property2', ESValue.wrap(undefined));
    propObj.setProperty(123, undefined);
    return propObj.getProperty('property1').isUndefined() &&
        propObj.getProperty('property2').isUndefined() &&
        propObj.getProperty(123).isUndefined();
}

function checkThrowNullOrUndefined(): boolean {
    let res = false;
    try {
        throwNull.invoke();
        res = false;
    } catch (e) {
        e = e as ESError;
        res = e.getValue().toNull() === null;
    }
    try {
        throwUndefined.invoke();
        res = false;
    } catch (e) {
        e = e as ESError;
        res = e.getValue().toUndefined() === undefined;
    }
    return res;
}

function testWrapNaN(): boolean {
    let nanVal: number = NaN;
    let esValue = ESValue.wrapNumber(nanVal);
    return esValue.isNumber() && esValue.typeOf() === 'number' && Number.isNaN(esValue.toNumber());
}

function testWrapInfinity(): boolean {
    let infVal: number = Infinity;
    let esValue = ESValue.wrapNumber(infVal);
    return esValue.isNumber() && esValue.typeOf() === 'number' && esValue.toNumber() === Infinity;
}

function testWrapNegativeInfinity(): boolean {
    let negInfVal: number = -Infinity;
    let esValue = ESValue.wrapNumber(negInfVal);
    return esValue.isNumber() && esValue.typeOf() === 'number' && esValue.toNumber() === -Infinity;
}

function testWrapMaxSafeInteger(): boolean {
    let maxVal: number = Number.MAX_SAFE_INTEGER;
    let esValue = ESValue.wrapNumber(maxVal);
    return esValue.isNumber() && esValue.typeOf() === 'number' && esValue.toNumber() === Number.MAX_SAFE_INTEGER;
}

function testWrapMinSafeInteger(): boolean {
    let minVal: number = Number.MIN_SAFE_INTEGER;
    let esValue = ESValue.wrapNumber(minVal);
    return esValue.isNumber() && esValue.typeOf() === 'number' && esValue.toNumber() === Number.MIN_SAFE_INTEGER;
}

function testWrapMaxValue(): boolean {
    let maxVal: number = Number.MAX_VALUE;
    let esValue = ESValue.wrapDouble(maxVal);
    return esValue.isNumber() && esValue.typeOf() === 'number' && esValue.toNumber() === Number.MAX_VALUE;
}

function testWrapMinValue(): boolean {
    let minVal: number = Number.MIN_VALUE;
    let esValue = ESValue.wrapDouble(minVal);
    return esValue.isNumber() && esValue.typeOf() === 'number' && esValue.toNumber() === Number.MIN_VALUE;
}

function testWrapZero(): boolean {
    let zeroVal: number = 0;
    let esValue = ESValue.wrapNumber(zeroVal);
    return esValue.isNumber() && esValue.toNumber() === 0;
}

function testWrapEmptyString(): boolean {
    let emptyStr: string = '';
    let esValue = ESValue.wrapString(emptyStr);
    return esValue.isString() && esValue.toString() === '';
}

function testWrapMaxBigInt(): boolean {
    let maxBigInt: bigint = 9007199254740991n;
    let esValue = ESValue.wrapBigInt(maxBigInt);
    return esValue.isBigInt() && esValue.toBigInt() === 9007199254740991n;
}

function testWrapByteMin(): boolean {
    let minByte: byte = -128;
    let esValue = ESValue.wrapByte(minByte);
    return esValue.isNumber() && esValue.unwrap() == -128;
}

function testWrapByteMax(): boolean {
    let maxByte: byte = 127;
    let esValue = ESValue.wrapByte(maxByte);
    return esValue.isNumber() && esValue.unwrap() == 127;
}

function testWrapShortMin(): boolean {
    let minShort: short = -32768;
    let esValue = ESValue.wrapShort(minShort);
    return esValue.isNumber() && esValue.unwrap() == -32768;
}

function testWrapShortMax(): boolean {
    let maxShort: short = 32767;
    let esValue = ESValue.wrapShort(maxShort);
    return esValue.isNumber() && esValue.unwrap() === 32767;
}

function testWrapIntMin(): boolean {
    let minInt: int = -2147483648;
    let esValue = ESValue.wrapInt(minInt);
    return esValue.isNumber() && esValue.toNumber() === -2147483648;
}

function testWrapIntMax(): boolean {
    let maxInt: int = 2147483647;
    let esValue = ESValue.wrapInt(maxInt);
    return esValue.isNumber() && esValue.toNumber() === 2147483647;
}

function testWrapLargeLong(): boolean {
    let largeLong: long = 9223372036854775807;
    let esValue = ESValue.wrapLong(largeLong);
    return esValue.isNumber() && esValue.unwrap() == 9223372036854775807;
}

function testWrapFloatMax(): boolean {
    let maxFloat: float = 3.4028235e38f;
    let esValue = ESValue.wrapFloat(maxFloat);
    return esValue.isNumber() && esValue.unwrap() == 3.4028235e38f;
}

function testToBooleanThrowsOnNumber(): boolean {
    let numVal = ESValue.wrapNumber(42);
    try {
        numVal.toBoolean();
        return false;
    } catch (e) {
        return e.message.includes('Boolean expected');
    }
}

function testToStringThrowsOnNumber(): boolean {
    let numVal = ESValue.wrapNumber(42);
    try {
        numVal.toString();
        return false;
    } catch (e) {
        return e.message.includes('String expected');
    }
}

function testToNumberThrowsOnString(): boolean {
    let strVal = ESValue.wrapString('hello');
    try {
        strVal.toNumber();
        return false;
    } catch (e) {
        return e.message.includes('Number expected');
    }
}

function testToBigIntThrowsOnNumber(): boolean {
    let numVal = ESValue.wrapNumber(42);
    try {
        numVal.toBigInt();
        return false;
    } catch (e) {
        return e.message.includes('bigint expected');
    }
}

function testToUndefinedThrowsOnNull(): boolean {
    let nullVal = ESValue.Null;
    try {
        nullVal.toUndefined();
        return false;
    } catch (e) {
        return e.message.includes('Can not cast to undefined');
    }
}

function testToNullThrowsOnUndefined(): boolean {
    let undefVal = ESValue.Undefined;
    try {
        undefVal.toNull();
        return false;
    } catch (e) {
        return e.message.includes('Can not cast to null');
    }
}

function testToStaticObjectThrowsOnECMAObject(): boolean {
    let jsObject = module.getProperty('A');
    try {
        jsObject.toStaticObject();
        return false;
    } catch (e) {
        return e.message.includes('Can not cast to a static object');
    }
}

function testInvokeMethodThrowsOnNonExistent(): boolean {
    let jsObject = module.getProperty('A');
    try {
        jsObject.invokeMethod('nonExistentMethod');
        return false;
    } catch (e) {
        return e.message.includes('failed to get property');
    }
}

function testIteratorThrowsOnNonIterable(): boolean {
    let jsObject = module.getProperty('A');
    try {
        for (const elem of jsObject) {
        }
        return false;
    } catch (e) {
        return e.message.includes('not iterable');
    }
}

function testToPromiseThrowsOnNonPromise(): boolean {
    let numVal = ESValue.wrapNumber(42);
    try {
        numVal.toPromise();
        return false;
    } catch (e) {
        return e.message.includes('Promise expected');
    }
}

function testWrapUndefined(): boolean {
    let wrapped = ESValue.wrap(undefined);
    return wrapped.isUndefined();
}

function testWrapNull(): boolean {
    let wrapped = ESValue.wrap(null);
    return wrapped.isNull();
}

class TestClass {
    value: number = 42;
}

function testWrapStaticObject(): boolean {
    let obj = new TestClass();
    let wrapped = ESValue.wrap(obj);
    return wrapped.isStaticObject();
}

function testUnwrap(): boolean {
    let original: number = 123;
    let wrapped = ESValue.wrapNumber(original);
    let unwrapped = wrapped.unwrap() as number;
    return unwrapped === original;
}

function testInstantiateEmptyObject(): boolean {
    let emptyObj = ESValue.instantiateEmptyObject();
    return emptyObj.isObject() && emptyObj.isECMAObject();
}

function testInstantiateEmptyArray(): boolean {
    let emptyArr = ESValue.instantiateEmptyArray();
    return emptyArr.isObject() && emptyArr.isECMAObject();
}

function testGetGlobal(): boolean {
    let global = ESValue.getGlobal();
    return global.isObject();
}

function testGetGlobalProperty(): boolean {
    let global = ESValue.getGlobal();
    let objectProp = global.getPropertySafe('Object');
    return objectProp.isFunction();
}

function testIsECMAObject(): boolean {
    let jsObject = module.getProperty('A');
    return jsObject.isECMAObject();
}

function testIsObjectWithStatic(): boolean {
    let obj = new TestClass();
    let wrapped = ESValue.wrap(obj);
    return wrapped.isObject();
}

function testIsObjectWithECMA(): boolean {
    let jsObject = module.getProperty('A');
    return jsObject.isObject();
}

function testAreEqualSafeNullNull(): boolean {
    let nullVal1 = ESValue.Null;
    let nullVal2 = ESValue.Null;
    return ESValue.areEqualSafe(nullVal1, nullVal2);
}

function testAreEqualSafeUndefUndef(): boolean {
    let undefVal1 = ESValue.Undefined;
    let undefVal2 = ESValue.Undefined;
    return ESValue.areEqualSafe(undefVal1, undefVal2);
}

function testAreEqualSafeNullUndef(): boolean {
    let nullVal = ESValue.Null;
    let undefVal = ESValue.Undefined;
    return ESValue.areEqualSafe(nullVal, undefVal);
}

function testIsEqualToSafe(): boolean {
    let nullVal = ESValue.Null;
    let undefVal = ESValue.Undefined;
    return nullVal.isEqualToSafe(undefVal);
}

function testIsPromise(): boolean {
    let jsPromise = Promise.resolve<Any>(undefined);
    let wrappedPromise = ESValue.wrap(jsPromise);
    return wrappedPromise.isPromise();
}

function testIsNotPromise(): boolean {
    let numVal = ESValue.wrapNumber(42);
    return !numVal.isPromise();
}

function testNegativeIndexAccess(): boolean {
    let jsArray = module.getProperty('jsArray');
    let negIndex = -1;
    let val = jsArray.getPropertySafe(negIndex);
    return val.isUndefined() || val.isNumber() || val.isString() || val.isBoolean();
}

function testLargeIndexAccess(): boolean {
    let jsArray = module.getProperty('jsArray');
    let largeIndex = 999999;
    let val = jsArray.getPropertySafe(largeIndex);
    return val.isUndefined();
}

function testNaNIndexAccess(): boolean {
    let jsObject = module.getProperty('A');
    let nanIndex = NaN;
    let val = jsObject.getPropertySafe(nanIndex);
    return val.isUndefined() || val.isNumber();
}

function testHasOwnProperty(): boolean {
    let jsObject = module.getProperty('A');
    return jsObject.hasOwnProperty('property1') &&
           !jsObject.hasOwnProperty('nonExistent');
}

function testHasOwnPropertyWithESValue(): boolean {
    let jsObject = module.getProperty('A');
    let propName = ESValue.wrapString('property1');
    return jsObject.hasOwnProperty(propName);
}

function testHasPropertyVsHasOwnProperty(): boolean {
    let jsArray = module.getProperty('jsArray');
    let hasOwn = jsArray.hasOwnProperty('length');
    let hasProp = jsArray.hasProperty('length');
    return hasOwn && hasProp;
}

function testInvokeWithRecv(): boolean {
    let jsFunc = module.getProperty('jsFunc');
    let customRecv = ESValue.instantiateEmptyObject();
    let result = jsFunc.invokeWithRecv(customRecv);
    return result.isNumber();
}

function testGetPropertySafeStringNonExistent(): boolean {
    let jsObject = module.getProperty('A');
    let val = jsObject.getPropertySafe('nonExistentProperty');
    return val.isUndefined();
}

function testGetPropertySafeNumberNonExistent(): boolean {
    let jsArray = module.getProperty('jsArray');
    let val = jsArray.getPropertySafe(999);
    return val.isUndefined();
}

function testGetPropertySafeESValueNonExistent(): boolean {
    let jsObject = module.getProperty('A');
    let prop = ESValue.wrapString('nonExistent');
    let val = jsObject.getPropertySafe(prop);
    return val.isUndefined();
}

function testSetPropertyToUndefined(): boolean {
    let obj = ESValue.instantiateEmptyObject();
    obj.setProperty('testProp', undefined);
    let val = obj.getProperty('testProp');
    return val.isUndefined();
}

function testSetPropertyToNull(): boolean {
    let obj = ESValue.instantiateEmptyObject();
    obj.setProperty('testProp', null);
    let val = obj.getProperty('testProp');
    return val.isNull();
}

function testSetPropertyWithESValue(): boolean {
    let obj = ESValue.instantiateEmptyObject();
    let value = ESValue.wrapNumber(123);
    obj.setProperty('testProp', value);
    let val = obj.getProperty('testProp');
    return val.isNumber() && val.toNumber() === 123;
}

function testWrapLongLossy(): boolean {
    let largeLong: long = 9007199254740993;
    let esValue = ESValue.wrapLongLossy(largeLong);
    return esValue.isNumber();
}

function testEmptyObjectIteration(): boolean {
    let emptyObj = ESValue.instantiateEmptyObject();
    let count = 0;
    for (const key of emptyObj.keys()) {
        count++;
    }
    return count === 0;
}

function testEmptyArrayIteration(): boolean {
    let emptyArr = ESValue.instantiateEmptyArray();
    let count = 0;
    for (const elem of emptyArr) {
        count++;
    }
    return count === 0;
}

function testComplexObjectNestedAccess(): boolean {
    let complexObj = module.getProperty('complexObject');

    let nested = complexObj.getProperty('nested');
    let level2 = nested.getProperty('level2');
    let deepValue = level2.getProperty('value');

    let arrayProp = complexObj.getProperty('array');
    let firstElem = arrayProp.getProperty(0);

    let funcProp = complexObj.getProperty('func');
    return deepValue.toString() === 'deep' &&
           firstElem.toNumber() === 1 &&
           funcProp.isFunction();
}

function testObjectWithMethodsInvocation(): boolean {
    let objWithMethods = module.getProperty('objectWithMethods');
    let result1 = objWithMethods.invokeMethod('method1');
    let args: FixedArray<ESValue> = [ESValue.wrapNumber(3), ESValue.wrapNumber(5)];
    let result2 = objWithMethods.invokeMethod('method2', ...args);
    let value = objWithMethods.getProperty('value');
    return result1.toString() === 'method1' &&
           result2.toNumber() === 8 &&
           value.toNumber() === 42;
}

function testObjWithNullsAccess(): boolean {
    let objWithNulls = module.getProperty('objWithNulls');
    let nullProp = objWithNulls.getProperty('nullProp');
    let undefProp = objWithNulls.getProperty('undefProp');
    let normalProp = objWithNulls.getProperty('normalProp');
    return nullProp.isNull() &&
           undefProp.isUndefined() &&
           normalProp.toString() === 'value';
}

function testJSClassInstantiation(): boolean {
    let TestClass = module.getProperty('TestClass');
    let args: FixedArray<ESValue> = [ESValue.wrapNumber(10), ESValue.wrapNumber(20)];
    let instance = TestClass.instantiate(...args);
    let sumResult = instance.invokeMethod('sum');
    return sumResult.toNumber() === 30;
}

function testJSClassInstanceProperties(): boolean {
    let TestClass = module.getProperty('TestClass');
    let args: FixedArray<ESValue> = [ESValue.wrapNumber(100), ESValue.wrapNumber(200)];
    let instance = TestClass.instantiate(...args);
    let x = instance.getProperty('x');
    let y = instance.getProperty('y');
    return x.toNumber() === 100 && y.toNumber() === 200;
}

function testCustomIterableManualNext(): boolean {
    let customIter = module.getProperty('customIterable');

    let global = ESValue.getGlobal();
    let symbol = global.getProperty("Symbol");
    let symbolIterator = symbol.getProperty("iterator");

    let iteratorMethod = customIter.getPropertySafe(symbolIterator);
    if (iteratorMethod.isUndefined()) {
        return false;
    }

    let iterator = iteratorMethod.invokeWithRecv(customIter);

    let next1 = iterator.invokeMethod("next");
    if (next1.getProperty("done").toBoolean()) {
        return false;
    }
    let value1 = next1.getProperty("value").toNumber();

    let next2 = iterator.invokeMethod("next");
    if (next2.getProperty("done").toBoolean()) {
        return false;
    }
    let value2 = next2.getProperty("value").toNumber();

    let next3 = iterator.invokeMethod("next");
    if (next3.getProperty("done").toBoolean()) {
        return false;
    }
    let value3 = next3.getProperty("value").toNumber();

    let next4 = iterator.invokeMethod("next");
    if (!next4.getProperty("done").toBoolean()) {
        return false;
    }

    return value1 === 10 && value2 === 20 && value3 === 30;
}

function testHasOwnPropertyVsHasPropertyWithProto(): boolean {
    let objWithProtoProps = module.getProperty('objWithProtoProps');

    let hasOwnOwn = objWithProtoProps.hasOwnProperty('own');
    let hasPropOwn = objWithProtoProps.hasProperty('own');

    let hasOwnInherited = objWithProtoProps.hasOwnProperty('inherited');
    let hasPropInherited = objWithProtoProps.hasProperty('inherited');

    return hasOwnOwn && hasPropOwn && hasOwnInherited && hasPropInherited;
}

function testHasOwnPropertyVsHasPropertyWithInheritance(): boolean {
    let Dog = module.getProperty('Dog');

    let dogInstance = Dog.instantiate(ESValue.wrapString('Buddy'), ESValue.wrapNumber(3), ESValue.wrapString('Golden Retriever'));

    let hasOwnName = dogInstance.hasOwnProperty('name');
    let hasPropName = dogInstance.hasProperty('name');
    let hasOwnAge = dogInstance.hasOwnProperty('age');
    let hasPropAge = dogInstance.hasProperty('age');
    let hasOwnBreed = dogInstance.hasOwnProperty('breed');
    let hasPropBreed = dogInstance.hasProperty('breed');

    return hasOwnName && hasPropName &&
           hasOwnAge && hasPropAge &&
           hasOwnBreed && hasPropBreed;
}

function testHasOwnPropertyOnClassConstructor(): boolean {
    let Animal = module.getProperty('Animal');
    let Dog = module.getProperty('Dog');

    let hasOwnName = Animal.hasOwnProperty('name');
    let hasPropName = Animal.hasProperty('name');

    let hasOwnPrototype = Dog.hasOwnProperty('prototype');
    let hasPropPrototype = Dog.hasProperty('prototype');

    let dogPrototype = Dog.getProperty('prototype');
    let hasOwnConstructor = dogPrototype.hasOwnProperty('constructor');
    let hasPropConstructor = dogPrototype.hasProperty('constructor');

    return hasOwnName && hasPropName &&
           hasOwnPrototype && hasPropPrototype &&
           hasOwnConstructor && hasPropConstructor;
}

function testHasOwnPropertyOnMethods(): boolean {
    let myDog = module.getProperty('myDog');

    let hasOwnSpeak = myDog.hasOwnProperty('speak');
    let hasPropSpeak = myDog.hasProperty('speak');

    let hasOwnGetInfo = myDog.hasOwnProperty('getInfo');
    let hasPropGetInfo = myDog.hasProperty('getInfo');

    let hasOwnFetch = myDog.hasOwnProperty('fetch');
    let hasPropFetch = myDog.hasProperty('fetch');

    return hasOwnSpeak && hasPropSpeak &&
           hasOwnGetInfo && hasPropGetInfo &&
           hasOwnFetch && hasPropFetch;
}

function testComplexObjectPropertyModification(): boolean {
    let complexObj = module.getProperty('complexObject');
    let nested = complexObj.getProperty('nested');

    nested.setProperty('newProp', ESValue.wrapString('new value'));
    let newProp = nested.getPropertySafe('newProp');
    return newProp.toString() === 'new value';
}

function testClassMethodOverride(): boolean {
    let myDog = module.getProperty('myDog');
    let speakResult = myDog.invokeMethod('speak');
    let infoResult = myDog.invokeMethod('getInfo');
    return speakResult.toString() === 'Buddy barks: Woof!' &&
           infoResult.toString() === 'Buddy is 3 years old and is a Golden Retriever';
}

function testClassUniqueMethod(): boolean {
    let myDog = module.getProperty('myDog');
    let fetchResult = myDog.invokeMethod('fetch');
    return fetchResult.toString() === 'Buddy fetches the ball';
}

function testClassInheritedMethod(): boolean {
    let myCat = module.getProperty('myCat');
    let infoResult = myCat.invokeMethod('getInfo');
    return infoResult.toString() === 'Whiskers is 2 years old';
}

function testClassSuperMethodCall(): boolean {
    let myCat = module.getProperty('myCat');
    let fullInfo = myCat.invokeMethod('getFullInfo');
    return fullInfo.toString() === 'Cat - Whiskers is 2 years old';
}

function testClassInstanceOfInheritance(): boolean {
    let Animal = module.getProperty('Animal');
    let Dog = module.getProperty('Dog');
    let myDog = module.getProperty('myDog');

    let isDog = myDog.instanceOf(Dog);
    let isAnimal = myDog.instanceOf(Animal);
    return isDog && isAnimal;
}

function testClassModifyInheritedFields(): boolean {
    let Dog = module.getProperty('Dog');

    let args: FixedArray<ESValue> = [
        ESValue.wrapString('Max'),
        ESValue.wrapNumber(4),
        ESValue.wrapString('Labrador')
    ];
    let maxDog = Dog.instantiate(...args);

    maxDog.setProperty('age', ESValue.wrapNumber(5));
    let age = maxDog.getProperty('age');
    let info = maxDog.invokeMethod('getInfo');

    return age.toNumber() === 5 &&
           info.toString() === 'Max is 5 years old and is a Labrador';
}

function testSymbolPropertyAccess(): boolean {
    let sym = module.getProperty('sym');
    let obj = module.getProperty('objWithSym');

    let global = ESValue.getGlobal();
    let symbolConstructor = global.getProperty('Symbol');

    let hasSym = obj.hasProperty(sym);
    let hasNormal = obj.hasProperty('normalProp');

    let normalProp = obj.getProperty('normalProp');
    return hasSym && hasNormal && normalProp.toString() === 'visible';
}

function testReflectHasProperty(): boolean {
    let target = module.getProperty('reflectTarget');
    let global = ESValue.getGlobal();
    let reflect = global.getProperty('Reflect');

    let hasX = reflect.invokeMethod('has', target, ESValue.wrapString('x'));
    let hasZ = reflect.invokeMethod('has', target, ESValue.wrapString('z'));

    return hasX.toBoolean() && !hasZ.toBoolean();
}

function testReflectOwnKeys(): boolean {
    let target = module.getProperty('reflectTarget');
    let global = ESValue.getGlobal();
    let reflect = global.getProperty('Reflect');

    let keys = reflect.invokeMethod('ownKeys', target);
    let length = keys.getProperty('length').toNumber();

    return length === 3;
}

function testReflectGetOwnPropertyDescriptor(): boolean {
    let target = module.getProperty('reflectTarget');
    let global = ESValue.getGlobal();
    let reflect = global.getProperty('Reflect');

    let descriptor = reflect.invokeMethod('getOwnPropertyDescriptor', target, ESValue.wrapString('x'));
    let hasValue = descriptor.hasProperty('value');

    return hasValue;
}

function testReflectApply(): boolean {
    let func = module.getProperty('reflectFunc');
    let global = ESValue.getGlobal();
    let reflect = global.getProperty('Reflect');

    let arrayCtor = global.getProperty('Array');
    let argsArray = arrayCtor.instantiate(
        ESValue.wrapNumber(1),
        ESValue.wrapNumber(2),
        ESValue.wrapNumber(3)
    );

    let result = reflect.invokeMethod('apply', func, ESValue.Undefined, argsArray);

    return result.toNumber() === 6;
}

function testObjectIsFrozenAndSealed(): boolean {
    let frozenObj = module.getProperty('frozenObj');
    let sealedObj = module.getProperty('sealedObj');
    let global = ESValue.getGlobal();
    let objectCtor = global.getProperty('Object');

    let isFrozen = objectCtor.invokeMethod('isFrozen', frozenObj);
    let isSealed = objectCtor.invokeMethod('isSealed', sealedObj);
    return isFrozen.toBoolean() && isSealed.toBoolean();
}

function testObjectIsExtensible(): boolean {
    let extensibleObj = module.getProperty('extensibleObj');
    let global = ESValue.getGlobal();
    let objectCtor = global.getProperty('Object');
    let isExtensible = objectCtor.invokeMethod('isExtensible', extensibleObj);
    return isExtensible.toBoolean();
}

function testFrozenObjectCannotModify(): boolean {
    let frozenObj = module.getProperty('frozenObj');
    let originalX = frozenObj.getProperty('x').toNumber();
    try {
        frozenObj.setProperty('x', ESValue.wrapNumber(999));
    } catch (e) {
        arktest.assertTrue(e.message === 'Cannot assign to read only property');
    }
    let currentX = frozenObj.getProperty('x').toNumber();
    return currentX === originalX;
}

function testFunctionBind(): boolean {
    let func = module.getProperty('funcToBind');
    let boundFunc = module.getProperty('funcBound');

    let args: FixedArray<ESValue> = [ESValue.wrapNumber(5)];
    let result = boundFunc.invoke(...args);

    return result.toNumber() === 115;
}

function testFunctionApply(): boolean {
    let func = module.getProperty('funcToApply');
    let context = module.getProperty('applyContext');
    let global = ESValue.getGlobal();
    let arrayCtor = global.getProperty('Array');
    let argsArray = arrayCtor.instantiate(
        ESValue.wrapNumber(1),
        ESValue.wrapNumber(10),
        ESValue.wrapNumber(100)
    );
    let result = func.invokeMethod('apply', context, argsArray);
    return result.toNumber() === 1111;
}

function testFunctionCall(): boolean {
    let func = module.getProperty('reflectTarget');
    let sumFunc = func.getProperty('sum');

    let args: FixedArray<ESValue> = [];
    let result = sumFunc.invokeWithRecv(func);

    return result.toNumber() === 30;
}

function testJsonStringify(): boolean {
    let obj = module.getProperty('nestedObj');
    let global = ESValue.getGlobal();
    let json = global.getProperty('JSON');

    let jsonString = json.invokeMethod('stringify', obj);
    let str = jsonString.toString();

    return str.indexOf('level1') !== -1 && str.indexOf('deep') !== -1;
}

function testJsonParse(): boolean {
    let global = ESValue.getGlobal();
    let json = global.getProperty('JSON');

    let jsonString = ESValue.wrapString('{"name":"test","value":42}');
    let parsed = json.invokeMethod('parse', jsonString);

    let name = parsed.getProperty('name').toString();
    let value = parsed.getProperty('value').toNumber();

    return name === 'test' && value === 42;
}

function testJsonSpecialValues(): boolean {
    let obj = module.getProperty('jsonSpecialValues');
    let global = ESValue.getGlobal();
    let json = global.getProperty('JSON');

    let jsonString = json.invokeMethod('stringify', obj);
    let str = jsonString.toString();

    return str.indexOf('text') !== -1 &&
           str.indexOf('123') !== -1 &&
           str.indexOf('true') !== -1 &&
           str.indexOf('null') !== -1;
}

function testJsonCircularReference(): boolean {
    let circularObj = module.getProperty('circularObj');
    let global = ESValue.getGlobal();
    let json = global.getProperty('JSON');

    let result = false;

    try {
        json.invokeMethod('stringify', circularObj);
    } catch (e) {
        arktest.assertTrue(e.message === 'stack contains value, usually caused by circular structure');
        result = true;
    }

    return result;
}

function testMapOperations(): boolean {
    let testMap = module.getProperty('testMap');
    let global = ESValue.getGlobal();
    let map = global.getProperty('Map');

    let size = testMap.invokeMethod('get', ESValue.wrapString('size'));
    if (size.isUndefined()) {
        size = testMap.getProperty('size');
    }

    let hasKey1 = testMap.invokeMethod('has', ESValue.wrapString('key1'));
    let value1 = testMap.invokeMethod('get', ESValue.wrapString('key1'));

    let hasNumKey = testMap.invokeMethod('has', ESValue.wrapNumber(123));
    let numValue = testMap.invokeMethod('get', ESValue.wrapNumber(123));

    return hasKey1.toBoolean() &&
           value1.toString() === 'value1' &&
           hasNumKey.toBoolean() &&
           numValue.toString() === 'number key';
}

function testSetOperations(): boolean {
    let testSet = module.getProperty('testSet');
    let size = testSet.getProperty('size');
    let has1 = testSet.invokeMethod('has', ESValue.wrapNumber(1));
    let has2 = testSet.invokeMethod('has', ESValue.wrapNumber(2));
    let has4 = testSet.invokeMethod('has', ESValue.wrapNumber(4));
    return size.toNumber() === 3 &&
           has1.toBoolean() &&
           has2.toBoolean() &&
           !has4.toBoolean();
}

function testWeakMapOperations(): boolean {
    let weakMap = module.getProperty('testWeakMap');
    let keysArr = module.getProperty('weakMapKeys');
    let key1 = keysArr.getPropertySafe(0);
    let has1 = weakMap.invokeMethod('has', key1);
    let value1 = weakMap.invokeMethod('get', key1);
    return has1.toBoolean() && value1.toString() === 'weak value 1';
}

function testWeakSetOperations(): boolean {
    let weakSet = module.getProperty('testWeakSet');
    let itemsArr = module.getProperty('weakSetItems');
    let item1 = itemsArr.getPropertySafe(0);
    let has1 = weakSet.invokeMethod('has', item1);
    let item2 = itemsArr.getPropertySafe(1);
    let has2 = weakSet.invokeMethod('has', item2);
    return has1.toBoolean() && has2.toBoolean();
}

function testSparseArray(): boolean {
    let sparseArr = module.getProperty('sparseArray');
    let elem0 = sparseArr.getPropertySafe(0);
    let elem1 = sparseArr.getPropertySafe(1);
    let elem3 = sparseArr.getPropertySafe(3);
    let length = sparseArr.getProperty('length');
    return elem0.toNumber() === 1 &&
           elem1.isUndefined() &&
           elem3.toNumber() === 4 &&
           length.toNumber() === 4;
}

function testArrayWithHoles(): boolean {
    let arr = module.getProperty('arrayWithHoles');
    let elem0 = arr.getPropertySafe(0);
    let elem4 = arr.getPropertySafe(4);
    let elem1 = arr.getPropertySafe(1);
    return elem0.toString() === 'first' &&
           elem4.toString() === 'last' &&
           elem1.isUndefined();
}

function testArrayLikeObject(): boolean {
    let obj = module.getProperty('arrayLikeObj');
    let elem0 = obj.getPropertySafe(0);
    let elem2 = obj.getPropertySafe(2);
    let length = obj.getProperty('length');
    return elem0.toString() === 'a' &&
           elem2.toString() === 'c' &&
           length.toNumber() === 3;
}

function testNonElementProperties(): boolean {
    let arr = module.getProperty('nonElementProperty');
    let elem0 = arr.getPropertySafe(0);
    let custom = arr.getProperty('custom');
    let negIndex = arr.getProperty(-1);
    let length = arr.getProperty('length');
    return elem0.toNumber() === 1 &&
           custom.toString() === 'not an index' &&
           negIndex.toString() === 'negative index' &&
           length.toNumber() === 3;
}

function testDateCreation(): boolean {
    let testDate = module.getProperty('testDate');
    let global = ESValue.getGlobal();
    let dateConstructor = global.getProperty('Date');
    let isDate = testDate.instanceOf(dateConstructor);
    return isDate;
}

function testDateGetTime(): boolean {
    let epochDate = module.getProperty('epochDate');
    let time = epochDate.invokeMethod('getTime');
    return time.toNumber() === 0;
}

function testInvalidDate(): boolean {
    let invalidDate = module.getProperty('invalidDate');
    let isValid = invalidDate.invokeMethod('toString').toString() !== 'Invalid Date';
    return !isValid;
}

function testDateToISOString(): boolean {
    let testDate = module.getProperty('testDate');
    let isoString = testDate.invokeMethod('toISOString').toString();
    return isoString.indexOf('2025-03-15') !== -1;
}

function testDateGetMethods(): boolean {
    let testDate = module.getProperty('testDate');
    let year = testDate.invokeMethod('getFullYear').toNumber();
    let month = testDate.invokeMethod('getMonth').toNumber();
    let date = testDate.invokeMethod('getDate').toNumber();
    return year === 2025 && month === 2 && date === 15;
}
