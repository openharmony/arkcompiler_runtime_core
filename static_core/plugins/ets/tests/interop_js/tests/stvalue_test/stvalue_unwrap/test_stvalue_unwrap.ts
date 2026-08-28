/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

const etsVm = globalThis.gtest.etsVm;

let STValue = etsVm.STValue;
let ns = STValue.findNamespace("stvalue_unwrap.Unwrap");
let stvalueWrap;
const epsilon = 1e-5;
let SType = etsVm.SType;

let userInfoCls = STValue.findClass('stvalue_unwrap.UserInfo');
let baseAnimalCls = STValue.findClass('stvalue_unwrap.BaseAnimal');
let dogBreedCls = STValue.findClass('stvalue_unwrap.DogBreed');

function testUnwrapToNumber(): void {
    stvalueWrap = STValue.wrapByte(1);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === 1);

    stvalueWrap = STValue.wrapChar('1');
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === 49); // ASCII code for '1'

    stvalueWrap = STValue.wrapShort(32767);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === 32767);

    stvalueWrap = STValue.wrapShort(-32768);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === -32768);

    stvalueWrap = STValue.wrapInt(44);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === 44);

    stvalueWrap = STValue.wrapLong(44);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === 44);


    stvalueWrap = STValue.wrapFloat(44.4);
    ASSERT_TRUE(Math.abs(stvalueWrap.unwrapToNumber() - 44.4) <= epsilon);

    stvalueWrap = STValue.wrapNumber(44.4);
    ASSERT_TRUE(Math.abs(stvalueWrap.unwrapToNumber() - 44.4) <= epsilon);

    stvalueWrap = STValue.wrapBoolean(true);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    let res = false;
    try {
        let magicSTValueNull = STValue.getNull();
        let magicNull = magicSTValueNull.unwrapToNumber();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type primitive");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueBigIntFromString = ns.namespaceGetVariable('bigIntFromString', SType.REFERENCE);
        let magicDouble = magicSTValueBigIntFromString.unwrapToNumber();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type primitive");
    }
    ASSERT_TRUE(res);
}

function testUnwrapToString(): void {
    let res = false;
    try {
        let magicSTValueNull = STValue.getNull();
        let magicNull = magicSTValueNull.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type std.core.String");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueUndefined = STValue.getUndefined();
        let magicUndefined = magicSTValueUndefined.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type std.core.String");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueBoolean = ns.namespaceGetVariable('magicBoolean', SType.BOOLEAN);
        let magicFloat = magicSTValueBoolean.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueByte = ns.namespaceGetVariable('magicByte', SType.BYTE);
        let magicByte = magicSTValueByte.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueChar = ns.namespaceGetVariable('magicChar', SType.CHAR);
        let magicChar = magicSTValueChar.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueShort = ns.namespaceGetVariable('magicShort', SType.SHORT);
        let magicShort = magicSTValueShort.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueInt = ns.namespaceGetVariable('magicInt', SType.INT);
        let magicInt = magicSTValueInt.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueLong = ns.namespaceGetVariable('magicLong', SType.LONG);
        let magicLong = magicSTValueLong.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueFloat = ns.namespaceGetVariable('magicFloat', SType.FLOAT);
        let magicFloat = magicSTValueFloat.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueDouble = ns.namespaceGetVariable('magicDouble', SType.DOUBLE);
        let magicDouble = magicSTValueDouble.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueBigIntFromString = ns.namespaceGetVariable('bigIntFromString', SType.REFERENCE);
        let magicDouble = magicSTValueBigIntFromString.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type std.core.String");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueBigIntFromLiteral = ns.namespaceGetVariable('bigIntFromLiteral', SType.REFERENCE);
        let magicDouble = magicSTValueBigIntFromLiteral.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type std.core.String");
    }
    ASSERT_TRUE(res);

    let magicSTValue1 = ns.namespaceGetVariable('magicString1', SType.REFERENCE);
    let magicString1 = magicSTValue1.unwrapToString();
    print('magicString1: ', magicString1);
    ASSERT_TRUE(magicString1 === 'Hello World');

    let magicSTValue2 = ns.namespaceGetVariable('magicString2', SType.REFERENCE);
    let magicString2 = magicSTValue2.unwrapToString();
    print('magicString2: ', magicString2);
    ASSERT_TRUE(magicString2 === 'Hello World!!!');

}

function testUnwrapToBigInt(): void {
    // testUnwrapToBigInt
    const bigIntFromStringValue = ns.namespaceGetVariable('bigIntFromString', SType.REFERENCE);
    const bigIntFromString = bigIntFromStringValue.unwrapToBigInt();

    ASSERT_TRUE(bigIntFromString === 999999999999999999n);
    ASSERT_TRUE(typeof bigIntFromString === 'bigint');

    const bigIntFromLiteralValue = ns.namespaceGetVariable('bigIntFromLiteral', SType.REFERENCE);
    const bigIntFromLiteral = bigIntFromLiteralValue.unwrapToBigInt();

    ASSERT_TRUE(bigIntFromLiteral === 123456789012345678901234567890n);
    ASSERT_TRUE(typeof bigIntFromLiteral === 'bigint');

    // testArithmeticOperations
    const bigIntValue = ns.namespaceGetVariable('bigIntFromString', SType.REFERENCE).unwrapToBigInt();

    const additionResult = bigIntValue + 1n;
    const multiplicationResult = bigIntValue * 2n;
    const subtractionResult = bigIntValue - 1000n;

    ASSERT_TRUE(additionResult === 1000000000000000000n);
    ASSERT_TRUE(multiplicationResult === 1999999999999999998n);
    ASSERT_TRUE(subtractionResult === 999999999999998999n);

    // testTypeCheck
    try {
        const numberValue = ns.namespaceGetVariable('defaultNumber', SType.REFERENCE);
        const result = numberValue.unwrapToBigInt();
        ASSERT_TRUE(false, 'Should have thrown error for non-BigInt conversion');
    } catch (error) {
        print('Expected error for non-BigInt conversion:', error.message);
        ASSERT_TRUE(true, 'Correctly threw error for non-BigInt conversion');
    }

    try {
        const stringVariable = ns.namespaceGetVariable('stringVariable', SType.REFERENCE);
        const result = stringVariable.unwrapToBigInt();
        print('Non-BigInt conversion result:', result);
        ASSERT_TRUE(false, 'Should have thrown error for non-BigInt conversion');
    } catch (error) {
        print('Expected error for non-BigInt conversion:', error.message);
        ASSERT_TRUE(true, 'Correctly threw error for non-BigInt conversion');
    }

    // testEdgeCases
    let zeroSTValue = ns.namespaceGetVariable('zeroBigInt', SType.REFERENCE);
    let zeroBigInt = zeroSTValue.unwrapToBigInt();

    ASSERT_TRUE(zeroBigInt === 0n);

    let negativeSTValue = ns.namespaceGetVariable('negativeBigInt', SType.REFERENCE);
    let negativeBigInt = negativeSTValue.unwrapToBigInt();

    ASSERT_TRUE(negativeBigInt === -1234567890123456789n);

    let res = false;
    try {
        let magicSTValueNull = STValue.getNull();
        magicSTValueNull.unwrapToBigInt();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("Expected BigInt object, but got different type.");
    }
    ASSERT_TRUE(res);
}
function testUnwrapToBoolean(): void {
    stvalueWrap = STValue.wrapByte(127);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    stvalueWrap = STValue.wrapByte(0);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === false);

    stvalueWrap = STValue.wrapByte(-44);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    stvalueWrap = STValue.wrapByte(-128);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true); // -128 = 0b(10000000) ?

    stvalueWrap = STValue.wrapInt(44);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    stvalueWrap = STValue.wrapInt(0);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === false);

    stvalueWrap = STValue.wrapInt(-44);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    stvalueWrap = STValue.wrapInt(-2147483648);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === false);

    stvalueWrap = STValue.wrapShort(32767);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    stvalueWrap = STValue.wrapShort(-32767);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    stvalueWrap = STValue.wrapShort(-32768);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === false); //-32768 = 0b(10000000 00000000)

    stvalueWrap = STValue.wrapLong(44);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    stvalueWrap = STValue.wrapLong(-44);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    stvalueWrap = STValue.wrapChar('1');
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true); // ASCII code for '1'

    stvalueWrap = STValue.wrapChar('0');
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true); // ASCII code for '0'

    stvalueWrap = STValue.wrapFloat(44.4);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    stvalueWrap = STValue.wrapFloat(0);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === false);

    stvalueWrap = STValue.wrapNumber(44.4);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    stvalueWrap = STValue.wrapNumber(0);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === false);

    let res = false;
    try {
        let magicSTValueNull = STValue.getNull();
        magicSTValueNull.unwrapToBoolean();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type primitive");
    }
    ASSERT_TRUE(res);
    print('UnwrapToBoolean Test SUCCESS!')
}

// ============================================================================
// JSON serialization tests (via classInvokeStaticMethod on std.core.JSON)
// ============================================================================

let JSONCls = STValue.findClass('std.core.JSON');

function stringifySTValue(stValue): string {
    return JSONCls.classInvokeStaticMethod('stringify',
        'C{std.core.Object}:C{std.core.String}', [stValue]).unwrapToString();
}

// Serialize flat objects with various property types
function testJSONStringifyFlatObjects(): void {
    let userInfo = ns.namespaceGetVariable('userInfo', SType.REFERENCE);
    let jsonStr = stringifySTValue(userInfo);
    ASSERT_TRUE(typeof jsonStr === 'string');
    ASSERT_TRUE(jsonStr.includes('Alice'));
    ASSERT_TRUE(jsonStr.includes('30'));
    ASSERT_TRUE(jsonStr.includes('95.5'));
    print('UserInfo JSON: ', jsonStr);

    let product = ns.namespaceGetVariable('productInfo', SType.REFERENCE);
    let productJson = stringifySTValue(product);
    ASSERT_TRUE(productJson.includes('Laptop'));
    ASSERT_TRUE(productJson.includes('1001'));

    let customUser = ns.namespaceGetVariable('customUserInfo', SType.REFERENCE);
    let customJson = stringifySTValue(customUser);
    ASSERT_TRUE(customJson.includes('Charlie'));
    ASSERT_TRUE(customJson.includes('28'));
}

// Serialize nested object (Person contains Address)
function testJSONStringifyNestedObject(): void {
    let person = ns.namespaceGetVariable('personInfo', SType.REFERENCE);
    let jsonStr = stringifySTValue(person);
    ASSERT_TRUE(typeof jsonStr === 'string');
    ASSERT_TRUE(jsonStr.includes('Bob'));
    ASSERT_TRUE(jsonStr.includes('25'));
    ASSERT_TRUE(jsonStr.includes('Shanghai'));
    ASSERT_TRUE(jsonStr.includes('200000'));
    print('Person JSON: ', jsonStr);
}

// Serialize empty class
function testJSONStringifyEmptyClass(): void {
    let emptyObj = ns.namespaceGetVariable('emptyObj', SType.REFERENCE);
    let jsonStr = stringifySTValue(emptyObj);
    ASSERT_TRUE(typeof jsonStr === 'string');
    print('EmptyClass JSON: ', jsonStr);
}

// Serialize object with array fields
function testJSONStringifyWithArrays(): void {
    let holder = ns.namespaceGetVariable('collectionHolder', SType.REFERENCE);
    let jsonStr = stringifySTValue(holder);
    ASSERT_TRUE(jsonStr.includes('tag1'));
    ASSERT_TRUE(jsonStr.includes('tag2'));
    ASSERT_TRUE(jsonStr.includes('42'));
    print('CollectionHolder JSON: ', jsonStr);

    let intArray = ns.namespaceGetVariable('intArray', SType.REFERENCE);
    let arrayJson = stringifySTValue(intArray);
    ASSERT_TRUE(arrayJson.includes('1'));
    ASSERT_TRUE(arrayJson.includes('5'));

    let emptyArray = ns.namespaceGetVariable('emptyArray', SType.REFERENCE);
    let emptyJson = stringifySTValue(emptyArray);
    ASSERT_TRUE(typeof emptyJson === 'string');
}

// Serialize object with all primitive field types (byte/short/int/long/float/double/char/boolean)
function testJSONStringifyPrimitiveFields(): void {
    let primitive = ns.namespaceGetVariable('primitiveFields', SType.REFERENCE);
    let jsonStr = stringifySTValue(primitive);
    ASSERT_TRUE(typeof jsonStr === 'string');
    ASSERT_TRUE(jsonStr.includes('100000'));
    ASSERT_TRUE(jsonStr.includes('1000'));
    print('PrimitiveFields JSON: ', jsonStr);
}

// Serialize object with nullable fields
function testJSONStringifyNullableFields(): void {
    let nullable = ns.namespaceGetVariable('nullableFields', SType.REFERENCE);
    let jsonStr = stringifySTValue(nullable);
    ASSERT_TRUE(typeof jsonStr === 'string');
    ASSERT_TRUE(jsonStr.includes('default'));
    print('NullableFields JSON: ', jsonStr);

    nullable.objectSetProperty('name', STValue.wrapString('SetNullable'), SType.REFERENCE);
    let afterSet = stringifySTValue(nullable);
    ASSERT_TRUE(afterSet.includes('SetNullable'));

    nullable.objectSetProperty('name', STValue.getNull(), SType.REFERENCE);
}

// Serialize object with enum field
function testJSONStringifyEnumField(): void {
    let enumHolder = ns.namespaceGetVariable('enumHolder', SType.REFERENCE);
    let jsonStr = stringifySTValue(enumHolder);
    ASSERT_TRUE(typeof jsonStr === 'string');
    ASSERT_TRUE(jsonStr.includes('holder'));
    print('EnumHolder JSON: ', jsonStr);
}

// Serialize object created via classInstantiate (not from namespace)
function testJSONStringifyClassInstantiate(): void {
    let instance = userInfoCls.classInstantiate('C{std.core.String}i:',
        [STValue.wrapString('David'), STValue.wrapInt(35)]);
    let jsonStr = stringifySTValue(instance);
    ASSERT_TRUE(jsonStr.includes('David'));
    ASSERT_TRUE(jsonStr.includes('35'));

    let defaultInstance = userInfoCls.classInstantiate(':', []);
    let defaultJson = stringifySTValue(defaultInstance);
    ASSERT_TRUE(defaultJson.includes('Alice'));
    ASSERT_TRUE(defaultJson.includes('30'));
}

// Serialize array created via newArray
function testJSONStringifyNewArray(): void {
    let intClass = STValue.findClass('std.core.Int');
    let intObj1 = intClass.classInstantiate('i:', [STValue.wrapInt(1)]);
    let intObj2 = intClass.classInstantiate('i:', [STValue.wrapInt(2)]);
    let newArray = STValue.newArray(2, intObj1);
    newArray.arraySet(1, intObj2);
    let jsonStr = stringifySTValue(newArray);
    ASSERT_TRUE(typeof jsonStr === 'string');
    print('NewArray JSON: ', jsonStr);
}

// Same object serialized twice produces identical result
function testJSONStringifyConsistency(): void {
    let userInfo = ns.namespaceGetVariable('userInfo', SType.REFERENCE);
    let json1 = stringifySTValue(userInfo);
    let json2 = stringifySTValue(userInfo);
    ASSERT_TRUE(json1 === json2);

    let sameObj = ns.namespaceGetVariable('userInfo', SType.REFERENCE);
    let json3 = stringifySTValue(sameObj);
    ASSERT_TRUE(json1 === json3);
}

// Different objects produce different JSON
function testJSONStringifyMultipleObjects(): void {
    let names = ['Alice', 'Bob', 'Charlie'];
    let ages = [30, 25, 28];
    let jsonStrings = [];
    for (let i = 0; i < names.length; i++) {
        let obj = userInfoCls.classInstantiate('C{std.core.String}i:',
            [STValue.wrapString(names[i]), STValue.wrapInt(ages[i])]);
        jsonStrings.push(stringifySTValue(obj));
    }
    for (let i = 0; i < names.length; i++) {
        ASSERT_TRUE(jsonStrings[i].includes(names[i]));
        ASSERT_TRUE(jsonStrings[i].includes(ages[i].toString()));
    }
    ASSERT_TRUE(jsonStrings[0] !== jsonStrings[1]);
    ASSERT_TRUE(jsonStrings[1] !== jsonStrings[2]);
}

// Modify object properties and verify JSON reflects changes
function testJSONStringifyAfterModification(): void {
    let userInfo = ns.namespaceGetVariable('userInfo', SType.REFERENCE);
    let jsonBefore = stringifySTValue(userInfo);
    ASSERT_TRUE(jsonBefore.includes('Alice'));

    userInfo.objectSetProperty('name', STValue.wrapString('ModifiedName'), SType.REFERENCE);
    userInfo.objectSetProperty('age', STValue.wrapInt(99), SType.INT);

    let jsonAfter = stringifySTValue(userInfo);
    ASSERT_TRUE(jsonAfter.includes('ModifiedName'));
    ASSERT_TRUE(jsonAfter.includes('99'));
    ASSERT_TRUE(!jsonAfter.includes('Alice'));
    ASSERT_TRUE(jsonBefore !== jsonAfter);

    userInfo.objectSetProperty('name', STValue.wrapString('Alice'), SType.REFERENCE);
    userInfo.objectSetProperty('age', STValue.wrapInt(30), SType.INT);
    let jsonRestored = stringifySTValue(userInfo);
    ASSERT_TRUE(jsonRestored.includes('Alice'));
    ASSERT_TRUE(jsonRestored.includes('30'));
}

// Call void method (setName) then verify state change in JSON
function testJSONStringifyAfterVoidMethod(): void {
    let userInfo = ns.namespaceGetVariable('userInfo', SType.REFERENCE);
    let jsonBefore = stringifySTValue(userInfo);
    ASSERT_TRUE(jsonBefore.includes('Alice'));

    userInfo.objectInvokeMethod('setName', 'C{std.core.String}:', [STValue.wrapString('VoidMethod')]);
    let jsonAfter = stringifySTValue(userInfo);
    ASSERT_TRUE(jsonAfter.includes('VoidMethod'));
    ASSERT_TRUE(!jsonAfter.includes('Alice'));

    userInfo.objectInvokeMethod('setName', 'C{std.core.String}:', [STValue.wrapString('Alice')]);
    let jsonRestored = stringifySTValue(userInfo);
    ASSERT_TRUE(jsonRestored.includes('Alice'));
}

// Serialize inherited object (DogBreed extends BaseAnimal)
function testJSONStringifyInheritance(): void {
    let dogObj = ns.namespaceGetVariable('dogObj', SType.REFERENCE);
    let jsonStr = stringifySTValue(dogObj);
    ASSERT_TRUE(typeof jsonStr === 'string');
    ASSERT_TRUE(jsonStr.includes('Labrador') || jsonStr.includes('animal'));

    let dogInstance = dogBreedCls.classInstantiate(':', []);
    ASSERT_TRUE(dogInstance.objectInstanceOf(dogBreedCls));
    ASSERT_TRUE(dogInstance.objectInstanceOf(baseAnimalCls));
    let dogJson = stringifySTValue(dogInstance);
    ASSERT_TRUE(typeof dogJson === 'string');
}

// Verify method return value matches JSON state
function testJSONStringifyWithMethodResult(): void {
    let person = ns.namespaceGetVariable('personInfo', SType.REFERENCE);
    let nameResult = person.objectInvokeMethod('getName', ':C{std.core.String}', []);
    ASSERT_TRUE(nameResult.unwrapToString() === 'Bob');
    let jsonStr = stringifySTValue(person);
    ASSERT_TRUE(jsonStr.includes('Bob'));
}

// Parse JSON result and verify structure
function testJSONStringifyParsedResult(): void {
    let userInfo = ns.namespaceGetVariable('userInfo', SType.REFERENCE);
    let jsonStr = stringifySTValue(userInfo);
    let parsed = JSON.parse(jsonStr);
    ASSERT_TRUE(parsed !== null && parsed !== undefined);
    ASSERT_TRUE(parsed.name === 'Alice');
    ASSERT_TRUE(parsed.age === 30);
    ASSERT_TRUE(parsed.active === true);
}

// ============================================================================
// toJSON (static) tests — works with proxy objects (shared references)
// ============================================================================

// toJSON with all three builtin proxy types
function testToJSONWithAllProxyTypes(): void {
    let stArr = STValue.newSTArray();
    let arrJson = STValue.toJSON(stArr);
    ASSERT_TRUE(typeof arrJson === 'string');
    print('st.Array toJSON: ', arrJson);

    let stMap = STValue.newSTMap();
    let mapJson = STValue.toJSON(stMap);
    ASSERT_TRUE(typeof mapJson === 'string');
    print('st.Map toJSON: ', mapJson);

    let stSet = STValue.newSTSet();
    let setJson = STValue.toJSON(stSet);
    ASSERT_TRUE(typeof setJson === 'string');
    print('st.Set toJSON: ', setJson);
}

// toJSON arg count errors
function testToJSONInvalidParamCount(): void {
    let checkRes = false;
    try {
        STValue.toJSON();
    } catch (e: Error) {
        checkRes = e.message.includes('Expect 1 args, but got 0 args');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        let userInfo = ns.namespaceGetVariable('userInfo', SType.REFERENCE);
        STValue.toJSON(userInfo, userInfo);
    } catch (e: Error) {
        checkRes = e.message.includes('Expect 1 args, but got 2 args');
    }
    ASSERT_TRUE(checkRes);
}

function main(): void {
    testUnwrapToNumber();
    testUnwrapToString();
    testUnwrapToBigInt();
    testUnwrapToBoolean();
    testJSONStringifyFlatObjects();
    testJSONStringifyNestedObject();
    testJSONStringifyEmptyClass();
    testJSONStringifyWithArrays();
    testJSONStringifyPrimitiveFields();
    testJSONStringifyNullableFields();
    testJSONStringifyEnumField();
    testJSONStringifyClassInstantiate();
    testJSONStringifyNewArray();
    testJSONStringifyConsistency();
    testJSONStringifyMultipleObjects();
    testJSONStringifyAfterModification();
    testJSONStringifyAfterVoidMethod();
    testJSONStringifyInheritance();
    testJSONStringifyWithMethodResult();
    testJSONStringifyParsedResult();
    testToJSONWithAllProxyTypes();
    testToJSONInvalidParamCount();
}

main();
