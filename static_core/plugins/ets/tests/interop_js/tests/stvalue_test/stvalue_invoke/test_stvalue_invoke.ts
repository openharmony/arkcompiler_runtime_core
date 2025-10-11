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

let SType = etsVm.SType;
let STValue = etsVm.STValue;
let module = STValue.findModule('stvalue_invoke');  

let studentCls = STValue.findClass('stvalue_invoke.Student');
let subStudentCls = STValue.findClass('stvalue_invoke.SubStudent');

// functionalObjectInvoke(args: Array<STValue>): STValue
function testFunctionalObjectInvoke(): void {
    let getNumberFn = module.moduleGetVariable('getNumberFn', SType.REFERENCE);
    let numRes = getNumberFn.functionalObjectInvoke([]);
    let jsNum = numRes.objectInvokeMethod('unboxed', ':i', []);
    print('jsNum:', jsNum.unwrapToNumber());
    ASSERT_TRUE(jsNum.unwrapToNumber() === 123);
}

// objectInvokeMethod(name: string, signature: string, args: Array<STValue>): STValue
function testObjectInvokeMethod(): void {
    let clsObj = studentCls.classInstantiate('iC{std.core.String}:', [STValue.wrapInt(18), STValue.wrapString('stu1')]);
    let stuAge = clsObj.objectInvokeMethod('getStudentAge', ':i', []);
    let stuName = clsObj.objectInvokeMethod('getStudentName', ':C{std.core.String}', []);
    clsObj.objectInvokeMethod('setStudentAge', 'i:', [STValue.wrapInt(20)]);
    let newStuAge = clsObj.objectInvokeMethod('getStudentAge', ':i', []);
    print('stuAge:', stuAge.unwrapToNumber(), '  stuName:', stuName.unwrapToString(), '  newAge:', newStuAge.unwrapToNumber());
    ASSERT_TRUE(stuAge.unwrapToNumber() === 18)
    ASSERT_TRUE(stuName.unwrapToString() === 'stu1')
    ASSERT_TRUE(newStuAge.unwrapToNumber() === 20)
}

// ClassInvokeStaticMethod(name: string, signature: string, args: Array<STValue>): STValue
function testClassInvokeStaticMethod(): void {
    let stuId = studentCls.classInvokeStaticMethod('getStudentId', ':i', []);
    studentCls.classInvokeStaticMethod('setStudentId', 'i:', [STValue.wrapInt(888)]);
    let newStuId = studentCls.classInvokeStaticMethod('getStudentId', ':i', []);
    print(stuId.unwrapToNumber(), newStuId.unwrapToNumber());
    ASSERT_TRUE(stuId.unwrapToNumber() === 999);
    ASSERT_TRUE(newStuId.unwrapToNumber() === 888);
}

function testModuleBooleanInvokeFunction():void{
    let mod = STValue.findModule('stvalue_invoke');
    let b1 = STValue.wrapBoolean(false);
    let b2 = STValue.wrapBoolean(false);
    let b = mod.moduleInvokeFunction('BooleanInvoke','zz:z',[b1,b2]);
    ASSERT_TRUE(b.unwrapToBoolean() === false);

    b1 = STValue.wrapBoolean(false);
    b2 = STValue.wrapBoolean(true);
    b = mod.moduleInvokeFunction('BooleanInvoke','zz:z',[b1,b2]);
    ASSERT_TRUE(b.unwrapToBoolean() === false);

    b1 = STValue.wrapBoolean(true);
    b2 = STValue.wrapBoolean(false);
    b = mod.moduleInvokeFunction('BooleanInvoke','zz:z',[b1,b2]);
    ASSERT_TRUE(b.unwrapToBoolean() === false);

    b1 = STValue.wrapBoolean(true);
    b2 = STValue.wrapBoolean(true);
    b = mod.moduleInvokeFunction('BooleanInvoke','zz:z',[b1,b2]);
    ASSERT_TRUE(b.unwrapToBoolean() === true);
}

function testModuleCharInvokeFunction():void{
    let mod = STValue.findModule('stvalue_invoke');
    let c = STValue.wrapChar('c');
    let r = mod.moduleInvokeFunction('CharInvoke','c:c',[c]);
    let charKlass = STValue.findClass('std.core.Char');
    let charObj = charKlass.classInstantiate('c:',[r]);
    let str = charObj.objectInvokeMethod('toString',':C{std.core.String}',[]);
    ASSERT_TRUE(str.unwrapToString() === 'c');
}

function testModuleByteInvokeFunction():void{
    let mod = STValue.findModule('stvalue_invoke');
    let b = STValue.wrapByte(112);
    let r = mod.moduleInvokeFunction('ByteInvoke','b:b',[b]);
    ASSERT_TRUE(r.unwrapToNumber() === 112);
}

function testModuleShortInvokeFunction():void{
    let mod = STValue.findModule('stvalue_invoke');
    let s = STValue.wrapShort(3456);
    let r = mod.moduleInvokeFunction('ShortInvoke','s:s',[s]);
    ASSERT_TRUE(r.unwrapToNumber() === 3456);
}

function testModuleIntInvokeFunction():void{
    let mod = STValue.findModule('stvalue_invoke');
     let a1 = STValue.wrapInt(123);
     let a2 = STValue.wrapInt(345);
    let r = mod.moduleInvokeFunction('IntInvoke','ii:i',[a1,a2]);
    ASSERT_TRUE(r.unwrapToNumber() === 468);
}

function testModuleLongInvokeFunction():void{
    let mod = STValue.findModule('stvalue_invoke');
     let a1 = STValue.wrapLong(123);
     let a2 = STValue.wrapLong(345);
    let r = mod.moduleInvokeFunction('LongInvoke','ll:l',[a1,a2]);
    ASSERT_TRUE(r.unwrapToNumber() === 468);
}

function testModuleFloatInvokeFunction():void{
    let mod = STValue.findModule('stvalue_invoke');
     let a1 = STValue.wrapFloat(1.4567);
     let a2 = STValue.wrapFloat(4.5678);
    mod.moduleInvokeFunction('FloatInvoke','ff:f',[a1,a2]);
}

function testModuleDoubleInvokeFunction():void{
    let mod = STValue.findModule('stvalue_invoke');
     let a1 = STValue.wrapNumber(1.4567);
     let a2 = STValue.wrapNumber(4.5678);
    mod.moduleInvokeFunction('DoubleInvoke','dd:d',[a1,a2]);
}

function testModuleNumberInvokeFunction():void{
    let mod = STValue.findModule('stvalue_invoke');
     let a1 = STValue.wrapNumber(1.4567);
     let a2 = STValue.wrapNumber(4.5678);
    mod.moduleInvokeFunction('NumberInvoke','dd:d',[a1,a2]);
}

function testModuleStringInvokeFunction():void{
    let mod = STValue.findModule('stvalue_invoke');
     let s1 = STValue.wrapString('ABCDEFG');
     let s2 = STValue.wrapString('HIJKLMN');
    let s = mod.moduleInvokeFunction('StringInvoke','C{std.core.String}C{std.core.String}:C{std.core.String}',[s1,s2]);
       ASSERT_TRUE(s.unwrapToString() === 'ABCDEFGHIJKLMN');
}

function testModuleBigIntInvokeFunction():void{
    let mod = STValue.findModule('stvalue_invoke');
     let b1 = STValue.wrapBigInt(BigInt('12345678'));
     let b2 = STValue.wrapBigInt(BigInt(12345678n));
    let s = mod.moduleInvokeFunction('BigIntInvoke','C{escompat.BigInt}C{escompat.BigInt}:C{escompat.BigInt}',[b1,b2]);
       ASSERT_TRUE(s.unwrapToBigInt() === BigInt(24691356n));
}

function testModuleVoidInvokeFunction():void{
    let mod = STValue.findModule('stvalue_invoke');
     let s1 = STValue.wrapString('ABCDEFG');
     let s2 = STValue.wrapString('HIJKLMN');
    mod.moduleInvokeFunction('VoidInvoke','C{std.core.String}C{std.core.String}:',[s1,s2]);
}

function testModuleInvokeFunction(): void {
    testModuleBooleanInvokeFunction();
    testModuleCharInvokeFunction();
    testModuleByteInvokeFunction();
    testModuleShortInvokeFunction();
    testModuleIntInvokeFunction();
    testNspLongInvokeFunction();
    testNspFloatInvokeFunction();
    testNspDoubleInvokeFunction();
    testNspNumberInvokeFunction();
    testNspStringInvokeFunction();
    testNspBigIntInvokeFunction();
    testNspVoidInvokeFunction();
}

function testNspBooleanInvokeFunction():void{
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let b1 = STValue.wrapBoolean(false);
    let b2 = STValue.wrapBoolean(false);
    let b = nsp.namespaceInvokeFunction('BooleanInvoke','zz:z',[b1,b2]);
    ASSERT_TRUE(b.unwrapToBoolean() === false);

    b1 = STValue.wrapBoolean(false);
    b2 = STValue.wrapBoolean(true);
    b = nsp.namespaceInvokeFunction('BooleanInvoke','zz:z',[b1,b2]);
    ASSERT_TRUE(b.unwrapToBoolean() === false);

    b1 = STValue.wrapBoolean(true);
    b2 = STValue.wrapBoolean(false);
    b = nsp.namespaceInvokeFunction('BooleanInvoke','zz:z',[b1,b2]);
    ASSERT_TRUE(b.unwrapToBoolean() === false);

    b1 = STValue.wrapBoolean(true);
    b2 = STValue.wrapBoolean(true);
    b = nsp.namespaceInvokeFunction('BooleanInvoke','zz:z',[b1,b2]);
    ASSERT_TRUE(b.unwrapToBoolean() === true);
}

function testNspCharInvokeFunction():void{
        let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let c = STValue.wrapChar('c');
    let r = nsp.namespaceInvokeFunction('CharInvoke','c:c',[c]);
    let charKlass = STValue.findClass('std.core.Char');
    let charObj = charKlass.classInstantiate('c:',[r]);
    let str = charObj.objectInvokeMethod('toString',':C{std.core.String}',[]);
    ASSERT_TRUE(str.unwrapToString() === 'c');
}

function testNspByteInvokeFunction():void{
        let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let b = STValue.wrapByte(112);
    let r = nsp.namespaceInvokeFunction('ByteInvoke','b:b',[b]);
    ASSERT_TRUE(r.unwrapToNumber() === 112);
}

function testNspShortInvokeFunction():void{
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let s = STValue.wrapShort(3456);
    let r = nsp.namespaceInvokeFunction('ShortInvoke','s:s',[s]);
    ASSERT_TRUE(r.unwrapToNumber() === 3456);
}

function testNspIntInvokeFunction():void{
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
     let a1 = STValue.wrapInt(123);
     let a2 = STValue.wrapInt(345);
    let r = nsp.namespaceInvokeFunction('IntInvoke','ii:i',[a1,a2]);
    ASSERT_TRUE(r.unwrapToNumber() === 468);
}

function testNspLongInvokeFunction():void{
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
     let a1 = STValue.wrapLong(123);
     let a2 = STValue.wrapLong(345);
    let r = nsp.namespaceInvokeFunction('LongInvoke','ll:l',[a1,a2]);
    ASSERT_TRUE(r.unwrapToNumber() === 468);
}

function testNspFloatInvokeFunction():void{
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
     let a1 = STValue.wrapFloat(1.4567);
     let a2 = STValue.wrapFloat(4.5678);
    nsp.namespaceInvokeFunction('FloatInvoke','ff:f',[a1,a2]);
}

function testNspDoubleInvokeFunction():void{
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
     let a1 = STValue.wrapNumber(1.4567);
     let a2 = STValue.wrapNumber(4.5678);
    nsp.namespaceInvokeFunction('DoubleInvoke','dd:d',[a1,a2]);
}

function testNspNumberInvokeFunction():void{
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
     let a1 = STValue.wrapNumber(1.4567);
     let a2 = STValue.wrapNumber(4.5678);
    nsp.namespaceInvokeFunction('NumberInvoke','dd:d',[a1,a2]);
}

function testNspStringInvokeFunction():void{
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
     let s1 = STValue.wrapString('ABCDEFG');
     let s2 = STValue.wrapString('HIJKLMN');
    let s = nsp.namespaceInvokeFunction('StringInvoke','C{std.core.String}C{std.core.String}:C{std.core.String}',[s1,s2]);
       ASSERT_TRUE(s.unwrapToString() === 'ABCDEFGHIJKLMN');
}

function testNspBigIntInvokeFunction():void{
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let b1 = STValue.wrapBigInt(BigInt('12345678'));
    let b2 = STValue.wrapBigInt(BigInt(12345678n));
    //let s = nsp.namespaceInvokeFunction('BigIntInvoke','C{escompat.BigInt}C{escompat.BigInt}:C{escompat.BigInt}',[b1,b2]);
    let s = nsp.namespaceInvokeFunction('BigIntInvoke','C{escompat.BigInt}C{escompat.BigInt}:C{escompat.BigInt}',[b1,b2]);
    ASSERT_TRUE(s.unwrapToBigInt() === BigInt(24691356n));
}

function testNspVoidInvokeFunction():void{
     let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
     let s1 = STValue.wrapString('ABCDEFG');
     let s2 = STValue.wrapString('HIJKLMN');
    nsp.namespaceInvokeFunction('VoidInvoke','C{std.core.String}C{std.core.String}:',[s1,s2]);
}

function testNamespaceInvokeFunction(): void {
    testNspBooleanInvokeFunction();
    testNspCharInvokeFunction();
    testNspByteInvokeFunction();
    testNspShortInvokeFunction();
    testNspIntInvokeFunction();
    testNspLongInvokeFunction();
    testNspFloatInvokeFunction();
    testNspDoubleInvokeFunction();
    testNspNumberInvokeFunction();
    testNspStringInvokeFunction();
    testNspBigIntInvokeFunction();
    testNspVoidInvokeFunction();
}

function main(): void {
    testFunctionalObjectInvoke();
    testObjectInvokeMethod();
    testClassInvokeStaticMethod();
    testModuleInvokeFunction();
    testNamespaceInvokeFunction();
}

main();
