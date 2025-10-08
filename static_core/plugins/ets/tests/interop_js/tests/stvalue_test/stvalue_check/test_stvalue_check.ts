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

const etsVm = globalThis.gtest.etsVm;

let STValue = etsVm.STValue;
let module = STValue.findModule('stvalue_check');
let SType = etsVm.SType;

let studentCls = STValue.findClass('stvalue_check.Student');
let subStudentCls = STValue.findClass('stvalue_check.SubStudent');

function testIsString(): void {
    let data = STValue.wrapInt(123);
    ASSERT_TRUE(data.isString() == false);
    
    data = STValue.wrapString('I am a Chinese');
    ASSERT_TRUE(data.isString() == true);
    
    data = module.moduleGetVariable("treeString", SType.REFERENCE);
    ASSERT_TRUE(data.isString() == true);

    data = module.moduleGetVariable("slicedString", SType.REFERENCE);
    ASSERT_TRUE(data.isString() == true);
}

function testIsBigInt(): void {
    let data = STValue.wrapInt(123);
    ASSERT_TRUE(data.isBigInt() == false);
    
    data = STValue.wrapBigInt(123456789n);
    ASSERT_TRUE(data.isBigInt() == true);
    
    data = STValue.wrapBigInt(BigInt('123456789'));
    ASSERT_TRUE(data.isBigInt() == true);
    
    data = module.moduleGetVariable("shouldBeBigInt", SType.REFERENCE);
    ASSERT_TRUE(data.isBigInt() == true);
}

function testIsBoolean(): void {
    let shouldBeBooleanTrue = module.moduleGetVariable("shouldBeBooleanTrue", SType.BOOLEAN);
    let shouldBeByte = module.moduleGetVariable("shouldBeByte", SType.BYTE);
    ASSERT_TRUE(shouldBeBooleanTrue.isBoolean());
    ASSERT_TRUE(!shouldBeByte.isBoolean());
}

function testIsByte(): void {
    let shouldBeByte = module.moduleGetVariable("shouldBeByte", SType.BYTE);
    let shouldBeBooleanTrue = module.moduleGetVariable("shouldBeBooleanTrue", SType.BOOLEAN);
    ASSERT_TRUE(shouldBeByte.isByte());
    ASSERT_TRUE(!shouldBeBooleanTrue.isByte());
}

function testIsChar(): void {
    let shouldBeChar = module.moduleGetVariable("shouldBeChar", SType.CHAR);
    let shouldBeByte = module.moduleGetVariable("shouldBeByte", SType.BYTE);
    ASSERT_TRUE(shouldBeChar.isChar());
    ASSERT_TRUE(!shouldBeByte.isChar());
}

function testIsShort(): void {
    let shouldBeShort = module.moduleGetVariable("shouldBeShort", SType.SHORT);
    let shouldBeChar = module.moduleGetVariable("shouldBeChar", SType.CHAR);
    ASSERT_TRUE(shouldBeShort.isShort());
    ASSERT_TRUE(!shouldBeChar.isShort());
}

function testIsInt(): void {
    let shouldBeInt = module.moduleGetVariable("shouldBeInt", SType.INT);
    let shouldBeShort = module.moduleGetVariable("shouldBeShort", SType.SHORT);
    ASSERT_TRUE(shouldBeInt.isInt());
    ASSERT_TRUE(!shouldBeShort.isInt());
}

function testIsLong(): void {
    let shouldBeLong = module.moduleGetVariable("shouldBeLong", SType.LONG);
    let shouldBeInt = module.moduleGetVariable("shouldBeInt", SType.INT);
    ASSERT_TRUE(shouldBeLong.isLong());
    ASSERT_TRUE(!shouldBeInt.isLong());
}

function testIsFloat(): void {
    let shouldBeFloat = module.moduleGetVariable("shouldBeFloat", SType.FLOAT);
    let shouldBeLong = module.moduleGetVariable("shouldBeLong", SType.LONG);
    ASSERT_TRUE(shouldBeFloat.isFloat());
    ASSERT_TRUE(!shouldBeLong.isFloat());
}

function testIsNumber(): void {
    let shouldBeNumber = module.moduleGetVariable("shouldBeNumber", SType.DOUBLE);
    let shouldBeFloat = module.moduleGetVariable("shouldBeFloat", SType.FLOAT);
    ASSERT_TRUE(shouldBeNumber.isNumber());
    ASSERT_TRUE(!shouldBeFloat.isNumber());
}

function testIsNullishValue(): void {
    let shouldBeNull = module.moduleGetVariable('shouldBeNull', SType.REFERENCE);
    let shouldBeUndefined = module.moduleGetVariable('shouldBeUndefined', SType.REFERENCE);
    let shouldBePrimitive = module.moduleGetVariable('shouldBePrimitive', SType.INT);
    let shouldBeRef = module.moduleGetVariable('shouldBeRef', SType.REFERENCE);

    ASSERT_TRUE(shouldBeNull.isNull());
    ASSERT_TRUE(shouldBeUndefined.isUndefined());
    ASSERT_TRUE(!shouldBeRef.isNull());
    ASSERT_TRUE(!shouldBeRef.isUndefined());
}

function testIsEqualTo(): void {
    let leftRef = module.moduleGetVariable('leftRef', SType.REFERENCE);
    let rightRef = module.moduleGetVariable('rightRef', SType.REFERENCE);
    ASSERT_TRUE(leftRef.isEqualTo(rightRef));

    let rightRefNotEqual = module.moduleGetVariable('rightRefNotEqual', SType.REFERENCE);
    ASSERT_TRUE(!leftRef.isEqualTo(rightRefNotEqual));

    let rightRefNotSameType = module.moduleGetVariable('rightRefNotSameType', SType.REFERENCE);
    ASSERT_TRUE(!leftRef.isEqualTo(rightRefNotSameType));

    let leftPrimitive = module.moduleGetVariable('leftPrimitive', SType.INT);
    let rightPrimitive = module.moduleGetVariable('rightPrimitive', SType.INT);
    let rightPrimitive2 = module.moduleGetVariable('rightPrimitive2', SType.INT);
    let rightPrimitive3 = module.moduleGetVariable('rightPrimitive3', SType.SHORT);
    let rightPrimitive4 = module.moduleGetVariable('rightPrimitive4', SType.LONG);

    let equalNull = STValue.getNull();
    let equalUndefined = STValue.getUndefined();
    ASSERT_TRUE(equalNull.isEqualTo(equalUndefined));
    ASSERT_TRUE(equalUndefined.isEqualTo(equalNull));
}

function testIsStrictlyEqualTo(): void {
    let magicInt = module.moduleGetVariable('magicInt', SType.INT);
    let magicDouble = module.moduleGetVariable('magicDouble', SType.DOUBLE);
    let magicString1 = module.moduleGetVariable('magicString1', SType.REFERENCE);
    let magicString2 = module.moduleGetVariable('magicString2', SType.REFERENCE);

    let result2 = magicString1.isStrictlyEqualTo(magicString2);
    print('Is magicString2 strictly equal to magicString1: ', result2);
    ASSERT_TRUE(result2 === false);

    let result3 = magicString1.isStrictlyEqualTo(magicString1);
    print('Is magicString1 strictly equal to magicString1: ', result3);
    ASSERT_TRUE(result3 === true);
}

// objectInstanceOf(typeArg: STValue): boolean
function testObjectInstanceOf(): void {
    let stuObj = studentCls.classInstantiate(':', []);
    let isInstance = stuObj.objectInstanceOf(studentCls);
    print('isInstance:', isInstance)
    ASSERT_EQ(isInstance, true);
}

// typeIsAssignableFrom(from_type: STValue, to_type: STValue): boolean
function testTypeIsAssignableFrom(): void {
    ASSERT_TRUE(STValue.typeIsAssignableFrom(subStudentCls, studentCls));
    ASSERT_TRUE(STValue.typeIsAssignableFrom(subStudentCls, subStudentCls))
    ASSERT_EQ(STValue.typeIsAssignableFrom(studentCls, subStudentCls), false)
    ASSERT_EQ(STValue.typeIsAssignableFrom(studentCls, module), false)
}

function main(): void {
    testIsString();
    testIsBigInt();
    testIsBoolean();
    testIsByte();
    testIsChar();
    testIsShort();
    testIsInt();
    testIsLong();
    testIsFloat();
    testIsNumber();
    testIsNullishValue();
    testIsEqualTo();
    testIsStrictlyEqualTo();
    testTypeIsAssignableFrom();
    testObjectInstanceOf();
}

main();
