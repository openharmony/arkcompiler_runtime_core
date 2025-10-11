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
    let stuObj = studentCls.classInstantiate(':', []);
    let boolArray = STValue.newFixedArrayPrimitive(5, SType.BOOLEAN);
    let res = stuObj.objectInvokeMethod('checkFixPrimitiveArray', 'A{z}:z', [boolArray]);
    ASSERT_TRUE(res.unwrapToBoolean());

    // let checkArray = module.moduleGetVariable('checkPrimitiveArrayFromDynamic', SType.REFERENCE);
    // let intClass = STValue.findClass('std.core.Int');
    // let intObj = intClass.classInstantiate('i:', [STValue.wrapInt(5)]);
    // let boolClass = STValue.findClass('std.core.Boolean');
    // let boolObj = boolClass.classInstantiate('z:', [STValue.wrapBoolean(false)]);

    // let booleanBoxedRes = checkArray.functionalObjectInvoke([boolArray]);
    // let res = booleanBoxedRes.objectInvokeMethod('unboxed', ':z', []);
}

// static newFixedArrayReference(len: number, elementType: STValue, initialElement: STValue): STValue
function testNewFixedArrayReference(): void {
    let stuObj = studentCls.classInstantiate(':', []);

    let intClass = STValue.findClass('std.core.Int');
    let intObj = intClass.classInstantiate('i:', [STValue.wrapInt(5)]);
    let refArray = STValue.newFixedArrayReference(3, intClass, intObj);

    let res = stuObj.objectInvokeMethod('checkFixReferenceArray', 'A{i}:z', [refArray]);
    ASSERT_TRUE(res.unwrapToBoolean());
}

// classFull.classInstantiateImpl(ctorSinature, array<Stvalue>): STValue
function testClassInstantiate(): void {
    let clsObj1 = studentCls.classInstantiate(':', []);
    let obj1Age = clsObj1.objectGetProperty('age', SType.INT).unwrapToNumber();
    let clsObj2 = studentCls.classInstantiate('i:', [STValue.wrapInt(23)]);
    let obj2Age = clsObj2.objectGetProperty('age', SType.INT).unwrapToNumber();
    print('obj1Age:', obj1Age, 'obj2Age:', obj2Age);
    ASSERT_EQ(obj1Age, 0);
    ASSERT_EQ(obj2Age, 23);
}

function main(): void {
    testNewFixedArrayPrimitive();
    testNewFixedArrayReference();
    testClassInstantiate();
}

main();
