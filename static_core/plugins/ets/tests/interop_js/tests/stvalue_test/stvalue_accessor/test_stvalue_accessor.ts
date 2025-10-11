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
let module = STValue.findModule('stvalue_accessor');

let SType = etsVm.SType;
let colorEnum = STValue.findEnum('stvalue_accessor.COLOR');
let intArray = module.moduleGetVariable('intArray', SType.REFERENCE);
let boolArray = module.moduleGetVariable('boolArray', SType.REFERENCE);
let strArray = module.moduleGetVariable('strArray', SType.REFERENCE);
let studentCls = STValue.findClass('stvalue_accessor.Student');

function testFixedArrayGetLength(): void {
    ASSERT_TRUE(strArray.fixedArrayGetLength() == 3);
    ASSERT_TRUE(boolArray.fixedArrayGetLength() == 4);
    ASSERT_TRUE(intArray.fixedArrayGetLength() === 5);
}

function testEnumAll(): void {
    ASSERT_TRUE(colorEnum.enumGetIndexByName('Red') === 0);
    ASSERT_TRUE(colorEnum.enumGetIndexByName('Green') === 1);
    ASSERT_TRUE(colorEnum.enumGetIndexByName('Blue') === 2);
    ASSERT_TRUE(colorEnum.enumGetIndexByName('Yellow') === 3);

    ASSERT_TRUE(colorEnum.enumGetNameByIndex(0) === 'Red');
    ASSERT_TRUE(colorEnum.enumGetNameByIndex(1) === 'Green');
    ASSERT_TRUE(colorEnum.enumGetNameByIndex(2) === 'Blue');
    ASSERT_TRUE(colorEnum.enumGetNameByIndex(3) === 'Yellow');

    // ASSERT_TRUE(colorEnum.enumGetValueByIndex(0, SType.INT).unwrapToNumber() === 1);  // TODO removed

    ASSERT_TRUE(colorEnum.enumGetValueByName('Red', SType.INT).unwrapToNumber() === 1);  // TODO
}

function testClassGetAndSetStaticField(): void {
    let personClass = STValue.findClass('stvalue_accessor.Person');
    ASSERT_TRUE(personClass.classGetStaticField('name', SType.REFERENCE).unwrapToString() === 'Person');
    ASSERT_TRUE(personClass.classGetStaticField('age', SType.INT).unwrapToNumber() === 18);
    ASSERT_TRUE(personClass.classGetStaticField('male', SType.BOOLEAN).unwrapToBoolean() === true);
    ASSERT_TRUE(personClass.classGetStaticField('id', SType.LONG).unwrapToNumber() === 1000000000);
    // ASSERT_TRUE(personClass.classGetStaticField('alpha', SType.CHAR) === 'a');
    ASSERT_TRUE(personClass.classGetStaticField('bits', SType.BYTE).unwrapToNumber() === 0x01);
    ASSERT_TRUE(personClass.classGetStaticField('height', SType.DOUBLE).unwrapToNumber() === 1.78);

    personClass.classSetStaticField('name', STValue.wrapString('Bob'), SType.REFERENCE);
    // personClass.classSetStaticField('alpha', 'b');
    personClass.classSetStaticField('height', STValue.wrapNumber(1.79), SType.DOUBLE);
    // personClass.classSetStaticField('grade', 5.11);
    personClass.classSetStaticField('id', STValue.wrapLong(20000000001), SType.LONG);
    personClass.classSetStaticField('age', STValue.wrapInt(28), SType.INT);
    personClass.classSetStaticField('bits', STValue.wrapByte(0x02), SType.BYTE);
    personClass.classSetStaticField('male', STValue.wrapBoolean(false), SType.BOOLEAN);

    ASSERT_TRUE(personClass.classGetStaticField('name', SType.REFERENCE).unwrapToString() === 'Bob');
    ASSERT_TRUE(personClass.classGetStaticField('age', SType.INT).unwrapToNumber() === 28);
    ASSERT_TRUE(personClass.classGetStaticField('male', SType.BOOLEAN).unwrapToBoolean() === false);
    // ASSERT_TRUE(personClass.classGetStaticField('alpha', SType.CHAR) === 'b');
    ASSERT_TRUE(personClass.classGetStaticField('bits', SType.BYTE).unwrapToNumber() === 0x02);
    ASSERT_TRUE(personClass.classGetStaticField('height', SType.DOUBLE).unwrapToNumber() === 1.79);
    ASSERT_TRUE(personClass.classGetStaticField('id', SType.LONG).unwrapToNumber() === 20000000001);
}

function testObjectGetAndSetProperty(): void {
    let studentInstance = module.moduleGetVariable('student', SType.REFERENCE);
    ASSERT_TRUE(studentInstance.objectGetProperty('name', SType.REFERENCE).unwrapToString() === 'Student');
    ASSERT_TRUE(studentInstance.objectGetProperty('age', SType.INT).unwrapToNumber() === 18);
    ASSERT_TRUE(studentInstance.objectGetProperty('male', SType.BOOLEAN).unwrapToBoolean() === true);
    // ASSERT_TRUE((studentInstance.objectGetProperty('grade', SType.FLOAT) - 3.14) <= 1e-5);
    // ASSERT_TRUE(studentInstance.objectGetProperty('alpha', SType.CHAR) === 'a');
    ASSERT_TRUE(studentInstance.objectGetProperty('bits', SType.BYTE).unwrapToNumber() === 0x01);
    ASSERT_TRUE(studentInstance.objectGetProperty('id', SType.LONG).unwrapToNumber() === 1000000000);
    ASSERT_TRUE(studentInstance.objectGetProperty('height', SType.DOUBLE).unwrapToNumber() === 1.78);

    studentInstance.objectSetProperty('name', STValue.wrapString('Ark'), SType.REFERENCE);
    studentInstance.objectSetProperty('age', STValue.wrapInt(28), SType.INT);
    // studentInstance.objectSetProperty('alpha', 'b');
    studentInstance.objectSetProperty('male', STValue.wrapBoolean(false), SType.BOOLEAN);
    studentInstance.objectSetProperty('id', STValue.wrapLong(1000000001), SType.LONG);
    studentInstance.objectSetProperty('bits', STValue.wrapByte(0x02), SType.BYTE);
    // studentInstance.objectSetProperty('grade', 2.14);
    studentInstance.objectSetProperty('height', STValue.wrapNumber(1.75), SType.DOUBLE);

    ASSERT_TRUE(studentInstance.objectGetProperty('name', SType.REFERENCE).unwrapToString() === 'Ark');
    ASSERT_TRUE(studentInstance.objectGetProperty('age', SType.INT).unwrapToNumber() === 28);
    ASSERT_TRUE(studentInstance.objectGetProperty('male', SType.BOOLEAN).unwrapToBoolean() === false);
    // ASSERT_TRUE((studentInstance.objectGetProperty('grade', SType.FLOAT).unwrapToFloat() - 2.14) <= 1e-5);
    // ASSERT_TRUE(studentInstance.objectGetProperty('alpha', SType.CHAR).toChar === 'b');
    ASSERT_TRUE(studentInstance.objectGetProperty('bits', SType.BYTE).unwrapToNumber() === 0x02);
    ASSERT_TRUE(studentInstance.objectGetProperty('id', SType.LONG).unwrapToNumber() === 1000000001);
    ASSERT_TRUE(studentInstance.objectGetProperty('height', SType.DOUBLE).unwrapToNumber() === 1.75);
}

function testFixedArrayGetAndSet(): void {
    ASSERT_TRUE(intArray.fixedArrayGet(0, SType.INT).unwrapToNumber() === 1);
    ASSERT_TRUE(boolArray.fixedArrayGet(3, SType.BOOLEAN).unwrapToBoolean() === false);
    ASSERT_TRUE(strArray.fixedArrayGet(1, SType.REFERENCE).unwrapToString() === 'cd');

    intArray.fixedArraySet(0, STValue.wrapInt(10), SType.INT);
    boolArray.fixedArraySet(3, STValue.wrapBoolean(true), SType.BOOLEAN);
    strArray.fixedArraySet(1, STValue.wrapString('xy'), SType.REFERENCE);

    ASSERT_TRUE(intArray.fixedArrayGet(0, SType.INT).unwrapToNumber() === 10);
    ASSERT_TRUE(boolArray.fixedArrayGet(3, SType.BOOLEAN).unwrapToBoolean() === true);
}

// Module variable get/set tests
function testModuleVariables() {
    // INT
    let magicSTValueInt = module.moduleGetVariable('magic_int', SType.INT);
    ASSERT_TRUE(magicSTValueInt.unwrapToNumber() == 42);
    module.moduleSetVariable('magic_int', STValue.wrapInt(44), SType.INT);
    ASSERT_TRUE(module.moduleGetVariable('magic_int', SType.INT).unwrapToNumber() == 44);

    // BOOLEAN
    let magicSTValueBool = module.moduleGetVariable('magic_boolean', SType.BOOLEAN);
    ASSERT_TRUE(magicSTValueBool.unwrapToNumber() == true);
    module.moduleSetVariable('magic_boolean', STValue.wrapBoolean(false), SType.BOOLEAN);
    ASSERT_TRUE(module.moduleGetVariable('magic_boolean', SType.BOOLEAN).unwrapToNumber() == false);

    // BYTE
    let magicSTValueByte = module.moduleGetVariable('magic_byte', SType.BYTE);
    ASSERT_TRUE(magicSTValueByte.unwrapToNumber() == 72);
    module.moduleSetVariable('magic_byte', STValue.wrapByte(74), SType.BYTE);
    ASSERT_TRUE(module.moduleGetVariable('magic_byte', SType.BYTE).unwrapToNumber() == 74);

    // SHORT
    let magicSTValueShort = module.moduleGetVariable('magic_short', SType.SHORT);
    ASSERT_TRUE(magicSTValueShort.unwrapToNumber() == 128);
    module.moduleSetVariable('magic_short', STValue.wrapShort(124), SType.SHORT);
    ASSERT_TRUE(module.moduleGetVariable('magic_short', SType.SHORT).unwrapToNumber() == 124);

    // LONG
    let magicSTValueLong = module.moduleGetVariable('magic_long', SType.LONG);
    ASSERT_TRUE(magicSTValueLong.unwrapToNumber() == 1024);
    module.moduleSetVariable('magic_long', STValue.wrapLong(1020), SType.LONG);
    ASSERT_TRUE(module.moduleGetVariable('magic_long', SType.LONG).unwrapToNumber() == 1020);

    // FLOAT
    let magicSTValueFloat = module.moduleGetVariable('magic_float', SType.FLOAT);
    ASSERT_TRUE(magicSTValueFloat.unwrapToNumber() - 3.14 < 0.01);
    module.moduleSetVariable('magic_float', STValue.wrapFloat(3.15), SType.FLOAT);
    ASSERT_TRUE(module.moduleGetVariable('magic_float', SType.FLOAT).unwrapToNumber() - 3.15 < 0.01);

    // DOUBLE
    let magicSTValueDouble = module.moduleGetVariable('magic_double', SType.DOUBLE);
    ASSERT_TRUE(magicSTValueDouble.unwrapToNumber() == 3.141592);
    module.moduleSetVariable('magic_double', STValue.wrapNumber(3.141593), SType.DOUBLE);
    ASSERT_TRUE(module.moduleGetVariable('magic_double', SType.DOUBLE).unwrapToNumber() == 3.141593);
}

/*
 * moduleGetSetVariableTest
 */
function testNamespaceVariable(): void {

    let ns = STValue.findNamespace('stvalue_accessor.magicNamespace');

    let magicSTValueInt = ns.namespaceGetVariable('magic_int_n', SType.INT);
    let magicNumberInt = magicSTValueInt.unwrapToNumber();
    ASSERT_TRUE(magicNumberInt == 42);
    ns.namespaceSetVariable('magic_int_n', STValue.wrapInt(44), SType.INT);
    let magicSTValueNewInt = ns.namespaceGetVariable('magic_int_n', SType.INT);
    let magicNumberNewInt = magicSTValueNewInt.unwrapToNumber();
    print('magicNumberNewInt', magicNumberNewInt);
    ASSERT_TRUE(magicNumberNewInt == 44);

    let magicSTValueBool = ns.namespaceGetVariable('magic_boolean_n', SType.BOOLEAN);
    let magicBool = magicSTValueBool.unwrapToNumber();
    ASSERT_TRUE(magicBool == true);
    ns.namespaceSetVariable('magic_boolean_n', STValue.wrapBoolean(false), SType.BOOLEAN);
    let magicSTValueNewBool = ns.namespaceGetVariable('magic_boolean_n', SType.BOOLEAN);
    let magicBoolNew = magicSTValueNewBool.unwrapToNumber();
    print('magicBoolNew', magicBoolNew);
    ASSERT_TRUE(magicBoolNew == false);

    let magicSTValueByte = ns.namespaceGetVariable('magic_byte_n', SType.BYTE);
    let magicByte = magicSTValueByte.unwrapToNumber();
    ASSERT_TRUE(magicByte == 72);
    ns.namespaceSetVariable('magic_byte_n', STValue.wrapByte(74), SType.BYTE);
    let magicSTValueNewByte = ns.namespaceGetVariable('magic_byte_n', SType.BYTE);
    let magicByteNew = magicSTValueNewByte.unwrapToNumber();
    print('magicByteNew', magicByteNew);
    ASSERT_TRUE(magicByteNew == 74);

    let magicSTValueShort = ns.namespaceGetVariable('magic_short_n', SType.SHORT);
    let magicShort = magicSTValueShort.unwrapToNumber();
    ASSERT_TRUE(magicShort == 128);
    ns.namespaceSetVariable('magic_short_n', STValue.wrapShort(124), SType.SHORT);
    let magicSTValueNewShort = ns.namespaceGetVariable('magic_short_n', SType.SHORT);
    let magicShortNew = magicSTValueNewShort.unwrapToNumber();
    print('magicShortNew', magicShortNew);
    ASSERT_TRUE(magicShortNew == 124);

    let magicSTValueLong = ns.namespaceGetVariable('magic_long_n', SType.LONG);
    let magicLong = magicSTValueLong.unwrapToNumber();
    ASSERT_TRUE(magicLong == 1024);
    ns.namespaceSetVariable('magic_long_n', STValue.wrapLong(1020), SType.LONG);
    let magicSTValueNewLong = ns.namespaceGetVariable('magic_long_n', SType.LONG);
    let magicLongNew = magicSTValueNewLong.unwrapToNumber();
    print('magicLongNew', magicLongNew);
    ASSERT_TRUE(magicLongNew == 1020);

    let magicSTValueFloat = ns.namespaceGetVariable('magic_float_n', SType.FLOAT);
    let magicFloat = magicSTValueFloat.unwrapToNumber();
    ASSERT_TRUE(Math.abs(magicFloat - 3.14) < 0.01);
    ns.namespaceSetVariable('magic_float_n', STValue.wrapFloat(3.15), SType.FLOAT);
    let magicSTValueNewFloat = ns.namespaceGetVariable('magic_float_n', SType.FLOAT);
    let magicFloatNew = magicSTValueNewFloat.unwrapToNumber();
    print('magicFloatNew', magicFloatNew);
    ASSERT_TRUE(Math.abs(magicFloatNew - 3.15) < 0.01);

    let magicSTValueDouble = ns.namespaceGetVariable('magic_double_n', SType.DOUBLE);
    let magicDouble = magicSTValueDouble.unwrapToNumber();
    ASSERT_TRUE(magicDouble == 3.141592);
    ns.namespaceSetVariable('magic_double_n', STValue.wrapNumber(3.141593), SType.DOUBLE);
    let magicSTValueNewDouble = ns.namespaceGetVariable('magic_double_n', SType.DOUBLE);
    let magicDoubleNew = magicSTValueNewDouble.unwrapToNumber();
    print('magicDoubleNew', magicDoubleNew);
    ASSERT_TRUE(magicDoubleNew == 3.141593);
    print('NP Success')
}

// objectGetType(): STValue
function testObjectGetType(): void {
    let stuObj = studentCls.classInstantiate(':', []);
    let stuType = stuObj.objectGetType();

    let strWrap = STValue.wrapString('string');
    let strType = strWrap.objectGetType();

    let biWrap = STValue.wrapBigInt(123n);
    let biTyep = biWrap.objectGetType();

    print(stuObj.objectInstanceOf(stuType), strWrap.objectInstanceOf(strType), biWrap.objectInstanceOf(biTyep));
    ASSERT_TRUE(stuObj.objectInstanceOf(stuType));
    ASSERT_TRUE(strWrap.objectInstanceOf(strType));
    ASSERT_TRUE(biWrap.objectInstanceOf(biTyep));
}

function testFindClass(): void {
    let klass = STValue.findClass('stvalue_accessor.Animal');
    ASSERT_TRUE(klass.classGetStaticField('name', SType.REFERENCE).unwrapToString() === 'Animal');
}

function testClassGetSuperClass(): void {
    let superClass = STValue.findClass('stvalue_accessor.Animal');
    let subClass = STValue.findClass('stvalue_accessor.Dog');
    let objectClass = STValue.findClass('std.core.Object');

    let objectSuperClass = objectClass.classGetSuperClass();
    ASSERT_TRUE(objectSuperClass === null);

    let resultClass: STValue = subClass.classGetSuperClass();
    ASSERT_TRUE(resultClass.classGetStaticField(
        'name', SType.REFERENCE).unwrapToString() === superClass.classGetStaticField('name', SType.REFERENCE).unwrapToString());
}


function stvalue_accessor() {
    testClassGetSuperClass();
    testFixedArrayGetLength();
    testEnumAll();
    testClassGetAndSetStaticField();
    testObjectGetAndSetProperty();
    testFixedArrayGetAndSet();
    testModuleVariables();
    testNamespaceVariable();
    testObjectGetType();
    testFindClass();
}

const result = stvalue_accessor();