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
let module = STValue.findModule('stvalue_instance');
let SType = etsVm.SType;

let studentCls = STValue.findClass('stvalue_instance.Student');
let subStudentCls = STValue.findClass('stvalue_instance.SubStudent');

// static newFixedArrayPrimitive(len: number, elementType: SType): STValue  boolean[]
function testNewFixedArrayPrimitive(): void {
    let checkRes = true;

    let boolArray = STValue.newFixedArrayPrimitive(5, SType.BOOLEAN);
    checkRes = checkRes && module.moduleInvokeFunction('checkFixPrimitiveBooleanArray', 'A{z}:z', [boolArray]).unwrapToBoolean();

    let byteArray = STValue.newFixedArrayPrimitive(5, SType.BYTE);
    checkRes = checkRes && module.moduleInvokeFunction('checkFixPrimitiveByteArray', 'A{b}:z', [byteArray]).unwrapToBoolean();

    let charArray = STValue.newFixedArrayPrimitive(5, SType.CHAR);
    checkRes = checkRes && module.moduleInvokeFunction('checkFixPrimitiveCharArray', 'A{c}:z', [charArray]).unwrapToBoolean();

    let shortArray = STValue.newFixedArrayPrimitive(5, SType.SHORT);
    checkRes = checkRes && module.moduleInvokeFunction('checkFixPrimitiveShortArray', 'A{s}:z', [shortArray]).unwrapToBoolean();

    let intArray = STValue.newFixedArrayPrimitive(5, SType.INT);
    checkRes = checkRes && module.moduleInvokeFunction('checkFixPrimitiveIntArray', 'A{i}:z', [intArray]).unwrapToBoolean();

    let longArray = STValue.newFixedArrayPrimitive(5, SType.LONG);
    checkRes = checkRes && module.moduleInvokeFunction('checkFixPrimitiveLongArray', 'A{l}:z', [longArray]).unwrapToBoolean();

    let floatArray = STValue.newFixedArrayPrimitive(5, SType.FLOAT);
    checkRes = checkRes && module.moduleInvokeFunction('checkFixPrimitiveFloatArray', 'A{f}:z', [floatArray]).unwrapToBoolean();

    let doubleArray = STValue.newFixedArrayPrimitive(5, SType.DOUBLE);
    checkRes = checkRes && module.moduleInvokeFunction('checkFixPrimitiveDoubleArray', 'A{d}:z', [doubleArray]).unwrapToBoolean();
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        STValue.newFixedArrayPrimitive(-1, SType.DOUBLE);
    } catch (e: Error) {
        checkRes = e.message.includes('ANI error occurred');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        STValue.newFixedArrayPrimitive(null, SType.DOUBLE);
    } catch (e: Error) {
        checkRes = e.message.includes('length type is not number type');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        STValue.newFixedArrayPrimitive(5, SType.REFERENCE);
    } catch (e: Error) {
        checkRes = e.message.includes('Unsupported SType: ');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        STValue.newFixedArrayPrimitive(5, SType.VOID); // SType.STREING
    } catch (e: Error) {
        checkRes = e.message.includes('Unsupported SType: ');
    }
    ASSERT_TRUE(checkRes);
}

// static newFixedArrayReference(len: number, elementType: STValue, initialElement: STValue): STValue
function testNewFixedArrayReference(): void {
    let intClass = STValue.findClass('std.core.Int');
    let intObj = intClass.classInstantiate('i:', [STValue.wrapInt(5)]);
    let refArray = STValue.newFixedArrayReference(3, intClass, intObj);
    let res = module.moduleInvokeFunction('checkFixReferenceObjectArray', 'A{C{std.core.Object}}:z', [refArray]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let undefArray = STValue.newFixedArrayReference(3, intClass, STValue.getUndefined());
    res = module.moduleInvokeFunction('checkFixReferenceUndefinedArray', 'A{C{std.core.Object}}:z', [undefArray]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let checkRes = false;
    try {
        STValue.newFixedArrayReference(-3, intClass, intObj);
    } catch (e: Error) {
        checkRes = e.message.includes('ANI error occurred');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        STValue.newFixedArrayReference(999, intClass, [STValue.getNull()]);
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('initialElement');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        STValue.newFixedArrayReference(999, intClass, STValue.wrapInt(123));
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('initialElement');
        checkRes = checkRes && e.message.includes('reference');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);
}

// classFull.classInstantiateImpl(ctorSinature, array<Stvalue>): STValue
function testClassInstantiate(): void {
    let clsObj1 = studentCls.classInstantiate(':', []);
    let obj1Age = clsObj1.objectGetProperty('age', SType.INT).unwrapToNumber();
    let clsObj2 = studentCls.classInstantiate('i:', [STValue.wrapInt(23)]);
    let obj2Age = clsObj2.objectGetProperty('age', SType.INT).unwrapToNumber();
    ASSERT_EQ(obj1Age, 0);
    ASSERT_EQ(obj2Age, 23);

    let res = false;
    try {
        studentCls.classInstantiate('i:i', []);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('ANI error occurred');
        res = res && e.message.includes('status: 7');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        studentCls.classInstantiate(':', STValue.wrapNumber(1));
        print('Not Error2');
    } catch (e: Error) {
        print('Error1:', e.message)
        res = true;
        res = res && e.message.includes('param are not array type');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        STValue.getUndefined().classInstantiate('i:', [STValue.wrapString('123')]);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('STValue instance does not wrap a value of type reference');
    }
    ASSERT_TRUE(res);

}

function main(): void {
    testNewFixedArrayPrimitive();
    testNewFixedArrayReference();
    testClassInstantiate();
}

main();
