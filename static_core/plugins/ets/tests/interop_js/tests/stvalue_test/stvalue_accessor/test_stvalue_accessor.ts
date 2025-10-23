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
let ns = STValue.findNamespace('stvalue_accessor.magicNamespace');

let SType = etsVm.SType;
let colorEnum = STValue.findEnum('stvalue_accessor.COLOR');
let mediaEnum = STValue.findEnum('stvalue_accessor.MEDIA');
let zeroArray = module.moduleGetVariable('zeroArray', SType.REFERENCE);
let intArray = module.moduleGetVariable('intArray', SType.REFERENCE);
let boolArray = module.moduleGetVariable('boolArray', SType.REFERENCE);
let strArray = module.moduleGetVariable('strArray', SType.REFERENCE);
let stuArray = module.moduleGetVariable('stuArray', SType.REFERENCE);
let studentCls = STValue.findClass('stvalue_accessor.Student');

function testFixedArrayGetLength(): void {
    ASSERT_TRUE(zeroArray.fixedArrayGetLength() === 0);
    ASSERT_TRUE(strArray.fixedArrayGetLength() === 3);
    ASSERT_TRUE(boolArray.fixedArrayGetLength() === 4);
    ASSERT_TRUE(intArray.fixedArrayGetLength() === 5);

    try {
        ASSERT_TRUE(colorEnum.fixedArrayGetLength() === 1);
    } catch (e: Error) { }

    try {
        strArray.fixedArrayGetLength(11);
    } catch (e: Error) { }
}

function testEnumAll(): void {
    ASSERT_TRUE(colorEnum.enumGetIndexByName('Red') === 0);
    ASSERT_TRUE(colorEnum.enumGetIndexByName('Green') === 1);
    ASSERT_TRUE(colorEnum.enumGetIndexByName('Blue') === 2);
    ASSERT_TRUE(colorEnum.enumGetIndexByName('Yellow') === 3);
    ASSERT_TRUE(mediaEnum.enumGetIndexByName('YAML') === 2);

    ASSERT_TRUE(colorEnum.enumGetNameByIndex(0) === 'Red');
    ASSERT_TRUE(colorEnum.enumGetNameByIndex(1) === 'Green');
    ASSERT_TRUE(colorEnum.enumGetNameByIndex(2) === 'Blue');
    ASSERT_TRUE(colorEnum.enumGetNameByIndex(3) === 'Yellow');
    ASSERT_TRUE(mediaEnum.enumGetNameByIndex(0) === 'JSON');

    ASSERT_TRUE(colorEnum.enumGetValueByName('Red', SType.INT).unwrapToNumber() === 1);
    ASSERT_TRUE(mediaEnum.enumGetValueByName('XML', SType.REFERENCE).unwrapToString() === 'app/xml');

    ASSERT_TRUE(colorEnum.enumGetValueByIndex(0, SType.INT).unwrapToNumber() === 1);
    ASSERT_TRUE(mediaEnum.enumGetValueByIndex(1, SType.REFERENCE).unwrapToString() === 'app/xml');

    try {
        colorEnum.enumGetIndexByName('Black', SType.INT);
    } catch (e: Error) { }

    try {
        colorEnum.enumGetIndexByName('Black');
    } catch (e: Error) { }

    try {
        colorEnum.enumGetNameByIndex(-1);
    } catch (e: Error) { }

    try {
        colorEnum.enumGetValueByName('Red', SType.REFERENCE);
    } catch (e: Error) { }

    try {
        let nullEmun = STValue.getNull();
        nullEnum.enumGetValueByName('Red', SType.REFERENCE);
    } catch (e: Error) { }
}

function testClassGetAndSetStaticField(): void {
    let personClass = STValue.findClass('stvalue_accessor.Person');
    ASSERT_TRUE(personClass.classGetStaticField('name', SType.REFERENCE).unwrapToString() === 'Person');
    ASSERT_TRUE(personClass.classGetStaticField('age', SType.INT).unwrapToNumber() === 18);
    ASSERT_TRUE(personClass.classGetStaticField('male', SType.BOOLEAN).unwrapToBoolean() === true);
    ASSERT_TRUE(personClass.classGetStaticField('id', SType.LONG).unwrapToNumber() === 1000000000);
    ASSERT_TRUE(personClass.classGetStaticField('bits', SType.BYTE).unwrapToNumber() === 0x01);
    ASSERT_TRUE(personClass.classGetStaticField('height', SType.DOUBLE).unwrapToNumber() === 1.78);

    personClass.classSetStaticField('name', STValue.wrapString('Bob'), SType.REFERENCE);
    personClass.classSetStaticField('height', STValue.wrapNumber(1.79), SType.DOUBLE);
    personClass.classSetStaticField('id', STValue.wrapLong(20000000001), SType.LONG);
    personClass.classSetStaticField('age', STValue.wrapInt(28), SType.INT);
    personClass.classSetStaticField('bits', STValue.wrapByte(0x02), SType.BYTE);
    personClass.classSetStaticField('male', STValue.wrapBoolean(false), SType.BOOLEAN);

    ASSERT_TRUE(personClass.classGetStaticField('name', SType.REFERENCE).unwrapToString() === 'Bob');
    ASSERT_TRUE(personClass.classGetStaticField('age', SType.INT).unwrapToNumber() === 28);
    ASSERT_TRUE(personClass.classGetStaticField('male', SType.BOOLEAN).unwrapToBoolean() === false);
    ASSERT_TRUE(personClass.classGetStaticField('bits', SType.BYTE).unwrapToNumber() === 0x02);
    ASSERT_TRUE(personClass.classGetStaticField('height', SType.DOUBLE).unwrapToNumber() === 1.79);
    ASSERT_TRUE(personClass.classGetStaticField('id', SType.LONG).unwrapToNumber() === 20000000001);

    try {
        personClass.classGetStaticField('xx', SType.INT);
    } catch (e: Error) { }
    try {
        personClass.classGetStaticField('male', SType.REFERENCE);
    } catch (e: Error) { }

    try {
        personClass.classSetStaticField('male', STValue.wrapBoolean(false), SType.INT);
    } catch (e: Error) { }

    try {
        personClass.classSetStaticField('male', STValue.wrapNumber(11), SType.BOOLEAN);
    } catch (e: Error) { }

    try {
        let nonClass = STValue.wrapInt(111);
        nonClass.classGetStaticField('male', SType.BOOLEAN);
    } catch (e: Error) { }

    try {
        personClass.classSetStaticField('pet', STValue.getNull(), SType.REFERENCE);
        ASSERT_TRUE(personClass.classGetStaticField('pet', SType.REFERENCE).isNull());
    } catch (e: Error) { }

    try {
        let nullClass = STValue.getNull();
        nullClass.classGetStaticField('male', SType.BOOLEAN);
    } catch (e: Error) { }

    try {
        personClass.classGetStaticField('male', SType.REFERENCE, STValue.getNull());
    } catch (e: Error) { }
}

function testObjectGetAndSetProperty(): void {
    let studentInstance = module.moduleGetVariable('student', SType.REFERENCE);
    ASSERT_TRUE(studentInstance.objectGetProperty('name', SType.REFERENCE).unwrapToString() === 'Student');
    ASSERT_TRUE(studentInstance.objectGetProperty('age', SType.INT).unwrapToNumber() === 18);
    ASSERT_TRUE(studentInstance.objectGetProperty('male', SType.BOOLEAN).unwrapToBoolean() === true);
    ASSERT_TRUE(studentInstance.objectGetProperty('bits', SType.BYTE).unwrapToNumber() === 0x01);
    ASSERT_TRUE(studentInstance.objectGetProperty('id', SType.LONG).unwrapToNumber() === 1000000000);
    ASSERT_TRUE(studentInstance.objectGetProperty('height', SType.DOUBLE).unwrapToNumber() === 1.78);
    ASSERT_TRUE(studentInstance.objectGetProperty('isChild', SType.BOOLEAN).unwrapToBoolean() === true);

    studentInstance.objectSetProperty('name', STValue.wrapString('Ark'), SType.REFERENCE);
    studentInstance.objectSetProperty('age', STValue.wrapInt(28), SType.INT);
    studentInstance.objectSetProperty('male', STValue.wrapBoolean(false), SType.BOOLEAN);
    studentInstance.objectSetProperty('id', STValue.wrapLong(1000000001), SType.LONG);
    studentInstance.objectSetProperty('bits', STValue.wrapByte(0x02), SType.BYTE);
    studentInstance.objectSetProperty('height', STValue.wrapNumber(1.75), SType.DOUBLE);
    studentInstance.objectSetProperty('isChild', STValue.wrapBoolean(false), SType.BOOLEAN);

    ASSERT_TRUE(studentInstance.objectGetProperty('name', SType.REFERENCE).unwrapToString() === 'Ark');
    ASSERT_TRUE(studentInstance.objectGetProperty('age', SType.INT).unwrapToNumber() === 28);
    ASSERT_TRUE(studentInstance.objectGetProperty('male', SType.BOOLEAN).unwrapToBoolean() === false);
    ASSERT_TRUE(studentInstance.objectGetProperty('bits', SType.BYTE).unwrapToNumber() === 0x02);
    ASSERT_TRUE(studentInstance.objectGetProperty('id', SType.LONG).unwrapToNumber() === 1000000001);
    ASSERT_TRUE(studentInstance.objectGetProperty('height', SType.DOUBLE).unwrapToNumber() === 1.75);
    ASSERT_TRUE(studentInstance.objectGetProperty('isChild', SType.BOOLEAN).unwrapToBoolean() === false);

    studentInstance.objectSetProperty('name', STValue.getNull(), SType.REFERENCE);
    ASSERT_TRUE(studentInstance.objectGetProperty('name', SType.REFERENCE).isNull());
    try {
        studentInstance.objectGetProperty('name', SType.INT);
    } catch (e: Error) { }

    try {
        studentInstance.objectGetProperty('xxx', SType.INT);
    } catch (e: Error) { }

    try {
        studentInstance.objectSetProperty('name', STValue.wrapNumber(111), SType.REFERENCE);
    } catch (e: Error) { }

    try {
        studentInstance.objectSetProperty('name', STValue.wrapString('Ark'), SType.INT);
    } catch (e: Error) { }

    try {
        let priObject = STValue.wrapInt(111);
        priObject.objectGetProperty('name', SType.REFERENCE);
    } catch (e: Error) { }

    try {
        let nullObject = STValue.getNull();
        nullObject.objectGetProperty('name', SType.REFERENCE);
    } catch (e: Error) { }

    try {
        studentInstance.objectSetProperty('name', STValue.wrapString('Ark'));
    } catch (e: Error) { }
}

function testFixedArrayGetAndSet(): void {
    ASSERT_TRUE(intArray.fixedArrayGet(0, SType.INT).unwrapToNumber() === 1);
    ASSERT_TRUE(boolArray.fixedArrayGet(3, SType.BOOLEAN).unwrapToBoolean() === false);
    ASSERT_TRUE(strArray.fixedArrayGet(1, SType.REFERENCE).unwrapToString() === 'cd');
    ASSERT_TRUE(stuArray.fixedArrayGet(0, SType.REFERENCE).objectGetProperty('age', SType.INT).unwrapToNumber() === 28);

    intArray.fixedArraySet(0, STValue.wrapInt(10), SType.INT);
    boolArray.fixedArraySet(3, STValue.wrapBoolean(true), SType.BOOLEAN);
    strArray.fixedArraySet(1, STValue.wrapString('xy'), SType.REFERENCE);

    ASSERT_TRUE(intArray.fixedArrayGet(0, SType.INT).unwrapToNumber() === 10);
    ASSERT_TRUE(boolArray.fixedArrayGet(3, SType.BOOLEAN).unwrapToBoolean() === true);

    try {
        strArray.fixedArraySet(0, STValue.getNull(), SType.REFERENCE);
        ASSERT_TRUE(strArray.fixedArrayGet(0, SType.REFERENCE).isNull());
    } catch (e: Error) { }

    try {
        intArray.fixedArrayGet(5, SType.INT);
    } catch (e: Error) { }

    try {
        intArray.fixedArrayGet(-1, SType.INT);
    } catch (e: Error) { }

    // try {
    //     intArray.fixedArrayGet(0, SType.REFERENCE);
    // } catch (e: Error) {
    //     print("ErrorTest: fixedArray gets wrong type: " + e.message);
    // }

    try {
        let nullArray = STValue.getNull();
        nullArray.fixedArraySet(0, STValue.wrapNumber(111), SType.FLOAT);
    } catch (e: Error) { }

    try {
        intArray.fixedArraySet(0, STValue.wrapNumber(111), SType.FLOAT);
    } catch (e: Error) { }

    try {
        intArray.fixedArraySet(0, STValue.wrapString('11'), SType.INT);
    } catch (e: Error) { }

    try {
        colorEnum.fixedArraySet(0, STValue.wrapNumber(111), SType.INT);
    } catch (e: Error) { }

    try {
        boolArray.fixedArrayGet(3, SType.BOOLEAN, STValue.wrapNumber(111));
    } catch (e: Error) { }

}

// Module variable get/set tests
function testModuleVariables() {
    ASSERT_TRUE(module.moduleGetVariable('magic_int', SType.INT).unwrapToNumber() === 42);
    ASSERT_TRUE(module.moduleGetVariable('magic_boolean', SType.BOOLEAN).unwrapToBoolean() === true);
    ASSERT_TRUE(module.moduleGetVariable('magic_byte', SType.BYTE).unwrapToNumber() === 72);
    ASSERT_TRUE(module.moduleGetVariable('magic_short', SType.SHORT).unwrapToNumber() === 128);
    ASSERT_TRUE(module.moduleGetVariable('magic_long', SType.LONG).unwrapToNumber() === 1024);
    ASSERT_TRUE(module.moduleGetVariable('magic_float', SType.FLOAT).unwrapToNumber() - 3.14 < 0.01);
    ASSERT_TRUE(Math.abs(module.moduleGetVariable('magic_double', SType.DOUBLE).unwrapToNumber() - 3.141592) < 0.000001);
    ASSERT_TRUE(module.moduleGetVariable('magic_float', SType.FLOAT).unwrapToNumber() - 3.14 < 0.01);

    module.moduleSetVariable('magic_int', STValue.wrapInt(44), SType.INT);
    module.moduleSetVariable('magic_boolean', STValue.wrapBoolean(false), SType.BOOLEAN);
    module.moduleSetVariable('magic_byte', STValue.wrapByte(74), SType.BYTE);
    module.moduleSetVariable('magic_short', STValue.wrapShort(124), SType.SHORT);
    module.moduleSetVariable('magic_long', STValue.wrapLong(1020), SType.LONG);
    module.moduleSetVariable('magic_float', STValue.wrapFloat(3.15), SType.FLOAT);
    module.moduleSetVariable('magic_double', STValue.wrapNumber(3.141593), SType.DOUBLE);

    ASSERT_TRUE(module.moduleGetVariable('magic_int', SType.INT).unwrapToNumber() === 44);
    ASSERT_TRUE(module.moduleGetVariable('magic_boolean', SType.BOOLEAN).unwrapToBoolean() === false);
    ASSERT_TRUE(module.moduleGetVariable('magic_byte', SType.BYTE).unwrapToNumber() === 74);
    ASSERT_TRUE(module.moduleGetVariable('magic_short', SType.SHORT).unwrapToNumber() === 124);
    ASSERT_TRUE(module.moduleGetVariable('magic_long', SType.LONG).unwrapToNumber() === 1020);
    ASSERT_TRUE(module.moduleGetVariable('magic_float', SType.FLOAT).unwrapToNumber() - 3.15 < 0.01);
    ASSERT_TRUE(Math.abs(module.moduleGetVariable('magic_double', SType.DOUBLE).unwrapToNumber() - 3.141593) < 0.000001);
}

function testModuleVariablesInvailidParam(): void {
    // variable not string type
    ASSERT_THROWS(Error, () => module.moduleGetVariable(null, SType.INT));
    ASSERT_THROWS(Error, () => module.moduleGetVariable(1, SType.INT));
    ASSERT_THROWS(Error, () => module.moduleSetVariable(null, STValue.wrapInt(44), SType.INT));
    ASSERT_THROWS(Error, () => module.moduleSetVariable(1, STValue.wrapInt(44), SType.INT));

    // variable not found
    ASSERT_THROWS(Error, () => module.moduleGetVariable('', SType.INT));
    ASSERT_THROWS(Error, () => module.moduleGetVariable('', SType.BOOLEAN));
    ASSERT_THROWS(Error, () => module.moduleGetVariable('', SType.BYTE));
    ASSERT_THROWS(Error, () => module.moduleGetVariable('', SType.SHORT));
    ASSERT_THROWS(Error, () => module.moduleGetVariable('', SType.LONG));
    ASSERT_THROWS(Error, () => module.moduleGetVariable('', SType.FLOAT));
    ASSERT_THROWS(Error, () => module.moduleGetVariable('', SType.DOUBLE));
    ASSERT_THROWS(Error, () => module.moduleSetVariable('', STValue.wrapBoolean(false), SType.BOOLEAN));
    ASSERT_THROWS(Error, () => module.moduleSetVariable('', STValue.wrapByte(74), SType.BYTE));
    ASSERT_THROWS(Error, () => module.moduleSetVariable('', STValue.wrapShort(74), SType.SHORT));
    ASSERT_THROWS(Error, () => module.moduleSetVariable('', STValue.wrapLong(1020), SType.LONG));
    ASSERT_THROWS(Error, () => module.moduleSetVariable('', STValue.wrapFloat(3.15), SType.FLOAT));
    ASSERT_THROWS(Error, () => module.moduleSetVariable('', STValue.wrapNumber(3.141593), SType.DOUBLE));
    ASSERT_THROWS(Error, () => module.moduleSetVariable('', STValue.wrapInt(44), SType.INT));

    // stype not match
    ASSERT_THROWS(Error, () => module.moduleGetVariable('magic_int', SType.DOUBLE));
    ASSERT_THROWS(Error, () => module.moduleGetVariable('magic_boolean', SType.BYTE));
    ASSERT_THROWS(Error, () => module.moduleGetVariable('magic_byte', SType.SHORT));
    ASSERT_THROWS(Error, () => module.moduleGetVariable('magic_short', SType.LONG));
    ASSERT_THROWS(Error, () => module.moduleGetVariable('magic_long', SType.FLOAT));
    ASSERT_THROWS(Error, () => module.moduleGetVariable('magic_float', SType.DOUBLE));
    ASSERT_THROWS(Error, () => module.moduleGetVariable('magic_double', SType.INT));
    ASSERT_THROWS(Error, () => module.moduleSetVariable('magic_int', STValue.wrapInt(44), SType.DOUBLE));
    ASSERT_THROWS(Error, () => module.moduleSetVariable('magic_boolean', STValue.wrapBoolean(false), SType.INT));
    ASSERT_THROWS(Error, () => module.moduleSetVariable('magic_byte', STValue.wrapByte(74), SType.BOOLEAN));
    ASSERT_THROWS(Error, () => module.moduleSetVariable('magic_short', STValue.wrapShort(124), SType.BYTE));
    ASSERT_THROWS(Error, () => module.moduleSetVariable('magic_long', STValue.wrapLong(1020), SType.SHORT));
    ASSERT_THROWS(Error, () => module.moduleSetVariable('magic_float', STValue.wrapFloat(3.15), SType.LONG));
    ASSERT_THROWS(Error, () => module.moduleSetVariable('magic_double', STValue.wrapNumber(3.141593), SType.FLOAT));

    // stvalue not match
    ASSERT_THROWS(Error, () => module.moduleSetVariable('magic_int', STValue.wrapBoolean(false), SType.INT));
    ASSERT_THROWS(Error, () => module.moduleSetVariable('magic_boolean', STValue.wrapByte(74), SType.BOOLEAN));
    ASSERT_THROWS(Error, () => module.moduleSetVariable('magic_byte', STValue.wrapShort(124), SType.BYTE));
    ASSERT_THROWS(Error, () => module.moduleSetVariable('magic_short', STValue.wrapLong(1020), SType.SHORT));
    ASSERT_THROWS(Error, () => module.moduleSetVariable('magic_long', STValue.wrapFloat(3.15), SType.LONG));
    ASSERT_THROWS(Error, () => module.moduleSetVariable('magic_float', STValue.wrapNumber(3.141593), SType.FLOAT));
    ASSERT_THROWS(Error, () => module.moduleSetVariable('magic_double', STValue.wrapInt(44), SType.DOUBLE));

    // invalid call
    ASSERT_THROWS(Error, () => STValue.wrapInt(44).moduleGetVariable('magic_int', SType.INT));
}

/*
 * moduleGetSetVariableTest
 */
function testNamespaceVariable(): void {
    ASSERT_TRUE(ns.namespaceGetVariable('magic_int_n', SType.INT).unwrapToNumber() === 42);
    ASSERT_TRUE(ns.namespaceGetVariable('magic_boolean_n', SType.BOOLEAN).unwrapToBoolean() === true);
    ASSERT_TRUE(ns.namespaceGetVariable('magic_byte_n', SType.BYTE).unwrapToNumber() === 72);
    ASSERT_TRUE(ns.namespaceGetVariable('magic_short_n', SType.SHORT).unwrapToNumber() === 128);
    ASSERT_TRUE(ns.namespaceGetVariable('magic_long_n', SType.LONG).unwrapToNumber() === 1024);
    ASSERT_TRUE(Math.abs(ns.namespaceGetVariable('magic_float_n', SType.FLOAT).unwrapToNumber() - 3.14) < 0.01);
    ASSERT_TRUE(Math.abs(ns.namespaceGetVariable('magic_double_n', SType.DOUBLE).unwrapToNumber() - 3.141592) < 0.000001);

    ns.namespaceSetVariable('magic_int_n', STValue.wrapInt(44), SType.INT);
    ns.namespaceSetVariable('magic_boolean_n', STValue.wrapBoolean(false), SType.BOOLEAN);
    ns.namespaceSetVariable('magic_byte_n', STValue.wrapByte(74), SType.BYTE);
    ns.namespaceSetVariable('magic_short_n', STValue.wrapShort(124), SType.SHORT);
    ns.namespaceSetVariable('magic_long_n', STValue.wrapLong(1020), SType.LONG);
    ns.namespaceSetVariable('magic_float_n', STValue.wrapFloat(3.15), SType.FLOAT);
    ns.namespaceSetVariable('magic_double_n', STValue.wrapNumber(3.141593), SType.DOUBLE);

    ASSERT_TRUE(ns.namespaceGetVariable('magic_int_n', SType.INT).unwrapToNumber() === 44);
    ASSERT_TRUE(ns.namespaceGetVariable('magic_boolean_n', SType.BOOLEAN).unwrapToBoolean() === false);
    ASSERT_TRUE(ns.namespaceGetVariable('magic_byte_n', SType.BYTE).unwrapToNumber() === 74);
    ASSERT_TRUE(ns.namespaceGetVariable('magic_short_n', SType.SHORT).unwrapToNumber() === 124);
    ASSERT_TRUE(ns.namespaceGetVariable('magic_long_n', SType.LONG).unwrapToNumber() === 1020);
    ASSERT_TRUE(Math.abs(ns.namespaceGetVariable('magic_float_n', SType.FLOAT).unwrapToNumber() - 3.15) < 0.01);
    ASSERT_TRUE(Math.abs(ns.namespaceGetVariable('magic_double_n', SType.DOUBLE).unwrapToNumber() - 3.141593) < 0.000001);
    print('NP Success')
}

function testNamespaceVariablesInvailidParam(): void {
    // variable not string type
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable(null, SType.INT));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable(1, SType.INT));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable(null, STValue.wrapInt(44), SType.INT));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable(1, STValue.wrapInt(44), SType.INT));

    // variable not found
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('', SType.INT));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('', SType.BOOLEAN));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('', SType.BYTE));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('', SType.SHORT));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('', SType.LONG));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('', SType.FLOAT));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('', SType.DOUBLE));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('', STValue.wrapBoolean(false), SType.BOOLEAN));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('', STValue.wrapByte(74), SType.BYTE));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('', STValue.wrapShort(74), SType.SHORT));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('', STValue.wrapLong(1020), SType.LONG));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('', STValue.wrapFloat(3.15), SType.FLOAT));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('', STValue.wrapNumber(3.141593), SType.DOUBLE));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('', STValue.wrapInt(44), SType.INT));

    // stype not match
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('magic_int_n', SType.DOUBLE));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('magic_boolean_n', SType.INT));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('magic_byte_n', SType.BOOLEAN));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('magic_short_n', SType.BYTE));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('magic_long_n', SType.SHORT));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('magic_float_n', SType.LONG));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('magic_double_n', SType.FLOAT));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_int_n', STValue.wrapInt(44), SType.DOUBLE));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_boolean_n', STValue.wrapBoolean(false), SType.INT));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_byte_n', STValue.wrapByte(74), SType.BOOLEAN));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_short_n', STValue.wrapShort(124), SType.BYTE));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_long_n', STValue.wrapLong(1020), SType.SHORT));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_float_n', STValue.wrapFloat(3.15), SType.LONG));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_double_n', STValue.wrapNumber(3.141593), SType.FLOAT));

    // stvalue not match
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_int_n', STValue.wrapBoolean(false), SType.INT));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_boolean_n', STValue.wrapByte(74), SType.BOOLEAN));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_byte_n', STValue.wrapShort(124), SType.BYTE));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_short_n', STValue.wrapLong(1020), SType.SHORT));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_long_n', STValue.wrapFloat(3.15), SType.LONG));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_float_n', STValue.wrapNumber(3.141593), SType.FLOAT));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_double_n', STValue.wrapInt(44), SType.DOUBLE));

    // invalid call
    ASSERT_THROWS(Error, () => STValue.wrapInt(44).namespaceGetVariable('magic_int', SType.INT));
}

// objectGetType(): STValue
function testObjectGetType(): void {
    let stuObj = studentCls.classInstantiate(':', []);
    let stuType = stuObj.objectGetType();

    let strWrap = STValue.wrapString('string');
    let strType = strWrap.objectGetType();

    let biWrap = STValue.wrapBigInt(123n);
    let biTyep = biWrap.objectGetType();

    ASSERT_TRUE(stuObj.objectInstanceOf(stuType));
    ASSERT_TRUE(strWrap.objectInstanceOf(strType));
    ASSERT_TRUE(biWrap.objectInstanceOf(biTyep));

    let checkRes = false;
    try {
        STValue.getUndefined().objectGetType();
        print('NOT Error')
    } catch (e) {
        print('hasError:', e.message)
        checkRes = true;
        checkRes = checkRes && e.message.includes('this');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);
}

function testFindClassInvalidParam(): void {
    // 1. no param
    let klass = null;
    try {
        klass = STValue.findClass();
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 2. param args not match
    try {
        klass = STValue.findClass('stvalue_accessor.Animal', 'stvalue_accessor.Animal');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.1 param not string type
    try {
        klass = STValue.findClass(null);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.2 param not string type
    try {
        klass = STValue.findClass(BigInt(123456n));
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.3 param not string type
    try {
        klass = STValue.findClass(1);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 4.1 empty param
    try {
        klass = STValue.findClass('');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 5.1 param contains special charactor
    try {
        klass = STValue.findClass('stvalue_accessor.Animal;');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 5.2 param contains special charactor
    try {
        klass = STValue.findClass('stvalue_accessor/Animal');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testFindClassNotFound(): void {
    let klass = null;
    try {
        klass = STValue.findClass('stvalue_accessor.NOT_FOUND');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testFindAbstractClass(): void {
    let absCls = STValue.findClass('stvalue_accessor.Shape');
    let cls = STValue.findClass('stvalue_accessor.Square');
    let superCls = cls.classGetSuperClass();
    ASSERT_TRUE(superCls.isStrictlyEqualTo(absCls));
}

function testFindFinalClass(): void {
    let finalCls = STValue.findClass('stvalue_accessor.City');
    let staticNameField = finalCls.classGetStaticField('name', SType.REFERENCE);
    ASSERT_TRUE(staticNameField.unwrapToString() === 'Shanghai');
}

function testFindClassInNsp(): void {
    let ccls = STValue.findClass('stvalue_accessor.Language.C');
    let staticNameField = ccls.classGetStaticField('name', SType.REFERENCE);
    ASSERT_TRUE(staticNameField.unwrapToString() === 'c language');
}

function testFindNestedClass(): void {
    let jscls = STValue.findClass('stvalue_accessor.Language.DynamicLanguage.Javascript');
    let staticNameField = jscls.classGetStaticField('name', SType.REFERENCE);
    ASSERT_TRUE(staticNameField.unwrapToString() === 'javascript language');
}

function testFindGenericClass(): void {
    let containerCls = STValue.findClass('stvalue_accessor.Container');
    let container = containerCls.classInstantiate(':', []);

    let intCls = STValue.findClass('std.core.Int');
    let data = STValue.wrapInt(100);
    let intObj = intCls.classInstantiate('i:', [data]);

    let area = container.objectInvokeMethod('calArea', 'C{std.core.Object}:C{std.core.Object}', [intObj]);
    let value = area.objectInvokeMethod('toInt', ':i', []);
    ASSERT_TRUE(value.unwrapToNumber() === 100);
}

function testFindClass(): void {
    testFindClassInvalidParam();
    testFindClassNotFound();
    testFindAbstractClass();
    testFindFinalClass();
    testFindClassInNsp();
    testFindNestedClass();
    testFindGenericClass();
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

    let nonClass = STValue.wrapInt(111);
    try {
        let resClass = nonClass.classGetSuperClass();
        ASSERT_TRUE(resClass === null);
    } catch (e: Error) { }
}

function testFindModuleInvalidParam(): void {
    // 1. no param
    let klass = null;
    try {
        klass = STValue.findModule();
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 2. param args not match
    try {
        klass = STValue.findModule('stvalue_accessor', 'stvalue_accessor');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.1 param not string type
    try {
        klass = STValue.findModule(null);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.2 param not string type
    try {
        klass = STValue.findModule(BigInt(123456n));
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.3 param not string type
    try {
        klass = STValue.findModule(1);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 4.1 empty param
    try {
        klass = STValue.findModule('');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 5.1 param contains special charactor
    try {
        klass = STValue.findModule('stvalue_accessor;');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 5.2 param contains special charactor
    try {
        klass = STValue.findModule('stvalue_accessor/');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testFindModuleNotFound(): void {
    let klass = null;
    try {
        klass = STValue.findModule('NOT_FOUND');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testFindModule(): void {
    testFindModuleInvalidParam();
    testFindModuleNotFound();
}

function testFindNamespaceInvalidParam(): void {
    // 1. no param
    let klass = null;
    try {
        klass = STValue.findNamespace();
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 2. param args not match
    try {
        klass = STValue.findNamespace('stvalue_accessor.Language', 'stvalue_accessor.Language');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.1 param not string type
    try {
        klass = STValue.findNamespace(null);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.2 param not string type
    try {
        klass = STValue.findNamespace(BigInt(123456n));
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.3 param not string type
    try {
        klass = STValue.findNamespace(1);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 4.1 empty param
    try {
        klass = STValue.findNamespace('');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 5.1 param contains special charactor
    try {
        klass = STValue.findNamespace('stvalue_accessor.Language;');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 5.2 param contains special charactor
    try {
        klass = STValue.findNamespace('stvalue_accessor/Language');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testFindNamespaceNotFound(): void {
    let klass = null;
    try {
        klass = STValue.findNamespace('stvalue_accessor.NOT_FOUND');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testCallFunctionInNamespace(): void {
    let nsp = STValue.findNamespace('stvalue_accessor.Language');
    let b1 = STValue.wrapBoolean(false);
    let b2 = STValue.wrapBoolean(true);
    let b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, b2]);
    ASSERT_TRUE(b.unwrapToBoolean() === false);
}

function testCallFunctionInNestedNamespace(): void {
    let nsp = STValue.findNamespace('stvalue_accessor.Language.DynamicLanguage');
    let b1 = STValue.wrapBoolean(false);
    let b2 = STValue.wrapBoolean(true);
    let b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, b2]);
    ASSERT_TRUE(b.unwrapToBoolean() === true);
}

function testCallFunctionInDifferentLevelNamespace(): void {
    let nsp = STValue.findNamespace('stvalue_accessor.Language.DynamicLanguage');
    let b1 = STValue.wrapBoolean(false);
    let b2 = STValue.wrapBoolean(true);
    let b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, b2]);
    ASSERT_TRUE(b.unwrapToBoolean() === true);

    nsp = STValue.findNamespace('stvalue_accessor.Language.DynamicLanguage.Python');
    b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, b2]);
    ASSERT_TRUE(b.unwrapToBoolean() === false);
}

function testFindNamespace(): void {
    testFindNamespaceInvalidParam();
    testFindNamespaceNotFound();
    testCallFunctionInNamespace();
    testCallFunctionInDifferentLevelNamespace();
}

function testFindEnumInvalidParam(): void {
    // 1. no param
    let klass = null;
    try {
        klass = STValue.findEnum();
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 2. param args not match
    try {
        klass = STValue.findEnum('stvalue_accessor.COLOR', 'stvalue_accessor.COLOR');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.1 param not string type
    try {
        klass = STValue.findEnum(null);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.2 param not string type
    try {
        klass = STValue.findEnum(BigInt(123456n));
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.3 param not string type
    try {
        klass = STValue.findEnum(1);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 4.1 empty param
    try {
        klass = STValue.findEnum('');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 5.1 param contains special charactor
    try {
        klass = STValue.findEnum('stvalue_accessor.COLOR;');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 5.2 param contains special charactor
    try {
        klass = STValue.findEnum('stvalue_accessor/COLOR');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testFindEnumNotFound(): void {
    let klass = null;
    try {
        klass = STValue.findEnum('stvalue_accessor.NOT_FOUND');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testFindEnum(): void {
    testFindEnumInvalidParam();
    testFindEnumNotFound();
    testEnumAll();
}

function stvalueAccessor() {
    testClassGetSuperClass();
    testFixedArrayGetLength();
    testClassGetAndSetStaticField();
    testObjectGetAndSetProperty();
    testFixedArrayGetAndSet();
    testModuleVariables();
    testModuleVariablesInvailidParam();
    testNamespaceVariable();
    testNamespaceVariablesInvailidParam();
    testObjectGetType();
    testFindClass();
    testFindModule();
    testFindNamespace();
    testFindEnum();
}

const result = stvalueAccessor();