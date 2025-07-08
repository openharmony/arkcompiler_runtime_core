/*
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

let module = ESObject.load("../../plugins/ets/tests/interop_js/tests/test_esobject/esobject_def_test");

let jsBoolean = module.getProperty("b");
let jsNumber = module.getProperty("n");
let jsString = module.getProperty("s");
let jsUndefined = module.getProperty("jsUndefined");
let jsBigInt = module.getProperty("bi");
let jsNull = module.getProperty("jsNull");
let jsFunc = module.getProperty("jsFunc");
let jsFuncNoReturn = module.getProperty("jsFuncNoReturn");
let jsFuncReturnNull = module.getProperty("jsFuncReturnNull");
let jsFuncReturnUndefined = module.getProperty("jsFuncReturnUndefined");
let jsFuncWithParam = module.getProperty("jsFuncWithParam");
let jsArray = module.getProperty("jsArray");
let jsArray1 = module.getProperty("jsArray1");
let jsObjectA = module.getProperty("A");
let jsObjectB = module.getProperty("B");
let jsObjWithFunc = module.getProperty("objWithFunc");
let jsObjWithFuncParam = module.getProperty("objWithFuncParam");
let jsIterable = module.getProperty("jsIterable");
let jsIterableObject = module.getProperty("testItetatorObject");
let jsUser = module.getProperty("User");
let jsDoubledObj = module.getProperty("doubledObj");

// get Undefined(): ESObject
function checkGetUndefined(): boolean {
    let obj = ESObject.Undefined;
    return obj.typeOf() == "undefined";
}

// get Null(): ESObject
function checkGetNull(): boolean {
    let obj = ESObject.Null;
    return obj.isNull();
}

// wrapBoolean(b: boolean): ESObject
function checkWrapBoolean(): boolean {
    let val = false;
    let obj = ESObject.wrapBoolean(val);
    return obj.toBoolean() == val;
}

// wrapString(s: string): ESObject
function checkWrapString(): boolean {
    let val = "hello";
    let obj = ESObject.wrapString(val);
    return obj.toString() == val;
}

// wrapBigInt(bi: BigInt): ESObject
function checkWrapBigInt(): boolean {
    let val = new BigInt(9007199254740991n);
    let obj = ESObject.wrapBigInt(val);
    return obj.toBigInt() == val;
}

// wrapNumber(n: number): ESObject
function checkWrapNumber(): boolean {
    let val = 111;
    let obj = ESObject.wrapNumber(val);
    return obj.toNumber() == val;
}

// wrapByte(b: byte): ESObject
function checkWrapByte(): boolean {
    let val: byte = 111;
    let obj = ESObject.wrapByte(val);
    return obj.toNumber() as byte == val;
}

// wrapShort(s: short): ESObject
function checkWrapShort(): boolean {
    let val: short = 111;
    let obj = ESObject.wrapShort(val);
    return obj.toNumber() as short == val;
}

// wrapInt(i: int): ESObject
function checkWrapInt(): boolean {
    let val: int = 111;
    let obj = ESObject.wrapInt(val);
    return obj.toNumber() as int == val;
}

// wrapLong(l: long): ESObject
function checkWrapLong(): boolean {
    let val: long = 111;
    let obj = ESObject.wrapLong(val);
    return obj.toNumber() as long == val;
}

// wrapFloat(f: float): ESObject
function checkWrapFloat(): boolean {
    let val: float = 111;
    let obj = ESObject.wrapFloat(val);
    return obj.toNumber() as float == val;
}

// wrapDouble(d: double): ESObject
function checkWrapDouble(): boolean {
    let val: double = 111;
    let obj = ESObject.wrapDouble(val);
    return obj.toNumber() as double == val;
}

// isBoolean(eo: ESObject): boolean
function checkIsBoolean(): boolean {
    return jsBoolean.isBoolean();
}

// isString(eo: ESObject): boolean
function checkIsString(): boolean {
    return jsString.isString();
}

// isNumber(eo: ESObject): boolean
function checkIsNumber(): boolean {
    return jsNumber.isNumber();
}

// isBigInt
function checkIsBigInt(): boolean {
    return jsBigInt.isBigInt();
}

// isUndefined
function checkIsUndefined(): boolean {
    let obj = ESObject.Undefined;
    return obj.isUndefined();
}

// isFunction
function checkIsFunction(): boolean {
    return jsFunc.isFunction();
}

// toBoolean(eo: ESObject): boolean
function checkToBoolean(): boolean {
    return jsBoolean.toBoolean() == false;
}

// toString(): string
function checkToString(): boolean {
    return jsString.toString() == "hello";
}

// toNumber(): number
function checkToNumber(): boolean {
    return jsNumber.toNumber() == 1;
}

// toBigInt(): bigint
function checkToBigInt(): boolean {
    return jsBigInt.toBigInt() == 9007199254740991n;
}

// toUndefined(): undefined
function checkToUndefined(): boolean {
    return jsUndefined.toUndefined() == undefined;
}

function checkJsIsUndefined(): boolean {
    return jsUndefined.isUndefined();
}

function checkTypeOfUndefined(): boolean {
    return jsUndefined.typeOf() == "undefined";
}

function checkStrictlyEqualUndefined(): boolean {
    return ESObject.areStrictlyEqual(jsUndefined, ESObject.Undefined);
}

// toNull(): null
function checkToNull(): boolean {
    return jsNull.toNull() == null;
}

function checkIsNull(): boolean {
    return jsNull.isNull();
}

function checkStrictlyEqualNull(): boolean {
    return ESObject.areStrictlyEqual(jsNull, ESObject.Null);
}

function checkAreStrictEqual(): boolean {
    return ESObject.areStrictlyEqual(jsFunc, jsFunc);
}

// isEqualTo(other: ESObject): boolean
function checkIsEqualTo(): boolean {
    return jsFunc.isEqualTo(jsFunc);
}

// isEqualToSafe(other: ESObject): boolean
function checkIsEqualToSafe(): boolean {
    let nullObj1 = ESObject.Null;
    let nullObj2 = ESObject.Null;
    return nullObj1.isEqualToSafe(nullObj2);
}

// getProperty(name: string): ESObject
function checkGetPropertyByName(): boolean {
    let val = jsObjectA.getProperty("property2");
    return val.toNumber() == 2;
}

class staticObjectA {
    aaa: string = ""
}
// getProperty(name: string): ESObject
function checkGetPropertyStaticObj(): boolean {
    let sObj = new staticObjectA();
    sObj.aaa = "bbb";
    let obj = ESObject.wrap(sObj);
    let value = obj.getPropertySafe("aaa");
    return value.toString() == "bbb";
}

// getPropertySafe(name: string): ESObject
function checkGetPropertyByNameSafe(): boolean {
    let val = jsObjectA.getPropertySafe("property3");
    return val.isUndefined() == true;
}

// getProperty(index: number): ESObject
function checkGetPropertyByIndex(): boolean {
    let val = jsArray.getProperty(2);
    return val.toBoolean() == true;
}

// getProperty(index: number): ESObject
function checkGetPropertyByIndexDouble(): boolean {
    let index = 3.2;
    let val = jsArray.getPropertySafe(3.2);
    let val1 = jsDoubledObj.getProperty(index);
    return val1.toString() == "aaa";
}

// getPropertySafe(index: number): ESObject
function checkGetPropertyByIndexSafe(): boolean {
    let val = jsArray.getPropertySafe(3);
    return val.isUndefined() == true;
}

// getProperty(property: ESObject): ESObject
function checkGetProperty(): boolean {
    let peroperty = ESObject.wrapString("property2");
    let val = jsObjectA.getProperty(peroperty);
    return val.toNumber() == 2;
}

// getPropertySafe(property: ESObject): ESObject
function checkGetPropertySafe(): boolean {
    let peroperty = ESObject.wrapString("property3");
    let val = jsObjectA.getPropertySafe(peroperty);
    return val.isUndefined() == true;
}

// setProperty(name: string, value: ESObject): void
function checkSetPropertyByName(): boolean {
    let value = ESObject.wrapNumber(5);
    jsObjectA.setProperty("property2", value);
    let val = jsObjectA.getProperty("property2");
    return val.toNumber() == 5;
}

// setProperty(index: number, value: StaticOrESObject): void
function checkSetPropertyByIndex(): boolean {
    let value = ESObject.wrapBoolean(false);
    jsArray1.setProperty(2, value);
    let val = jsArray1.getProperty(2);
    return val.toBoolean() == false;
}

// setProperty(property: ESObject, value: ESObject): void
function checkSetProperty(): boolean {
    let value = ESObject.wrapNumber(5);
    let property = ESObject.wrapString("property2");
    jsObjectA.setProperty(property, value);
    let val = jsObjectA.getProperty(property);
    return val.toNumber() == 5;
}

// hasProperty(property: ESObject): boolean
function checkHasProperty(): boolean {
    let property = ESObject.wrapString("property2");
    return jsObjectB.hasProperty(property) == true;
}

// hasProperty(name: String): boolean
function checkHasPropertyByName(): boolean {
    return jsObjectB.hasProperty("property2");
}

// typeOf(): String
function checkTypeOf(): boolean {
    let jsType = jsString.typeOf();
    return jsType == "string"
}

// public invoke(...args: ESObject[]): ESObject
function checkInvokeNoParam(): boolean {
    let result = jsFunc.invoke();
    if (!result.isNumber()) {
        return false;
    }
    return result.toNumber() == 6;
}

function checkInvokeNoReturn(): boolean {
    let result = jsFuncNoReturn.invoke();
    return result.isUndefined();
}

function checkInvokeReturnNull(): boolean {
    let result = jsFuncReturnNull.invoke();
    return result.isNull();
}

function checkInvokeReturnUndefined(): boolean {
    let result = jsFuncReturnUndefined.invoke();
    return result.isUndefined();
}

// public invoke(...args: FixedArray<ESObject>): ESObject
function checkInvokeHasParam(): boolean {
    let args = [ESObject.wrapDouble(2), ESObject.wrapDouble(3)];
    let result = jsFuncWithParam.invoke(...args);
    if (!result.isNumber()) {
        return false;
    }
    return result.toNumber() == 5;
}

function checkInvokeMethod(): boolean {
    let result = jsObjWithFunc.invokeMethod("func");
    if (!result.isString()) {
        return false;
    }
    return result.toString() == "hello";
}

function checkInvokeMethodHasParam(): boolean {
    let args = [ESObject.wrapString("abc"), ESObject.wrapString("def")];
    let result = jsObjWithFuncParam.invokeMethod("hello", ...args);
    if (!result.isString()) {
        return false;
    }
    return result.toString() == "abcdef";
}

function checkIterator(): boolean {
    let result: String = new String();
    for (const elem of jsIterable) {
        result += elem.toString();
    }
    return result == "abcd";
}

function checkKeys(): boolean {
    let keys = ["a", "b", "c"];
    let index = 0;
    for (const key of jsIterableObject.keys()) {
        if (key.toString() != keys[index]) {
            return false;
        }
        index++;
    }
    if (index != 3) {
        return false;
    }
    return true;
}

function checkValues(): boolean {
    let values = [1, 2, 3];
    let index = 0;
    for (const value of jsIterableObject.values()) {
        if (value.toNumber() != values[index]) {
            return false;
        }
        index++;
    }
    if (index != 3) {
        return false;
    }
    return true;
}

function checkEntries(): boolean {
    let keys = ["a", "b", "c"];
    let values = [1, 2, 3];
    let index = 0;
    for (const entry of jsIterableObject.entries()) {
        if (entry[0].toString() != keys[index]) {
            return false;
        }
        if (entry[1].toNumber() != values[index]) {
            return false;
        }
        index++;
    }
    if (index != 3) {
        return false;
    }
    return true;
}

class testObj {};
function checkInstanceOfStaticObj(): boolean {
    let staticObj = new testObj();
    let obj = ESObject.wrap(staticObj);
    return obj.instanceOf(Type.of(staticObj));
}

function checkInstanceOfNumeric(): boolean {
    let num: short = 3;
    let obj = ESObject.wrapNumber(num);
    return obj.instanceOf(Type.of(num));
}

function checkInstanceOfStaticPrimitive(): boolean {
    let boolObj = false;
    let obj = ESObject.wrapBoolean(boolObj);
    return obj.instanceOf(Type.of(boolObj));
}

function checkInstanceOfDynamic(): boolean {
	let num = ESObject.wrapInt(123);
	let user = jsUser.instantiate(num);
    return user.instanceOf(jsUser);
}

function checkInstaniate(): boolean {
	let num = ESObject.wrapInt(123);
	let user = jsUser.instantiate(num);
    return user.getProperty("ID").toNumber() == 123;
}
