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
let SType = etsVm.SType;
const ITERATIONS = 500;

// findClass and findNamespace tests
function testFindClassStdCore(): void {
  for (let i = 0; i < ITERATIONS; i++) {
    let cls = STValue.findClass('std.core.Int');
  }
}

function testFindClassUserDefined(): void {
  for (let i = 0; i < ITERATIONS; i++) {
    let cls = STValue.findClass('stvalue_workload.TestClass');
  }
}

function testFindNamespace(): void {
  for (let i = 0; i < ITERATIONS; i++) {
    let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  }
}

function testFindClassMultiple(): void {
  let classes = [
    'std.core.Int',
    'std.core.String',
    'std.core.Boolean',
    'std.core.Double',
    'std.core.Array'
  ];
  for (let i = 0; i < ITERATIONS; i++) {
    for (let j = 0; j < classes.length; j++) {
      let cls = STValue.findClass(classes[j]);
    }
  }
}

// classInstantiate tests
function testClassInstantiateNoArgs(): void {
  let cls = STValue.findClass('stvalue_workload.TestClass');
  for (let i = 0; i < ITERATIONS; i++) {
    let obj = cls.classInstantiate(':', []);
  }
}

function testClassInstantiateWithArgs(): void {
  let cls = STValue.findClass('std.core.Int');
  for (let i = 0; i < ITERATIONS; i++) {
    let obj = cls.classInstantiate('i:', [STValue.wrapInt(42)]);
  }
}

// namespaceGetVariable and namespaceSetVariable tests
function testNamespaceGetString50(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  for (let i = 0; i < ITERATIONS; i++) {
    let val = ns.namespaceGetVariable('testField50', SType.REFERENCE);
  }
}

function testNamespaceGetString100(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  for (let i = 0; i < ITERATIONS; i++) {
    let val = ns.namespaceGetVariable('testField100', SType.REFERENCE);
  }
}

function testNamespaceGetString500(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  for (let i = 0; i < ITERATIONS; i++) {
    let val = ns.namespaceGetVariable('testField500', SType.REFERENCE);
  }
}

function testNamespaceGetString1000(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  for (let i = 0; i < ITERATIONS; i++) {
    let val = ns.namespaceGetVariable('testField1000', SType.REFERENCE);
  }
}

function testNamespaceGetString5000(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  for (let i = 0; i < ITERATIONS; i++) {
    let val = ns.namespaceGetVariable('testField5000', SType.REFERENCE);
  }
}

function testNamespaceGetString10000(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  for (let i = 0; i < ITERATIONS; i++) {
    let val = ns.namespaceGetVariable('testField10000', SType.REFERENCE);
  }
}

function testNamespaceGetString50000(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  for (let i = 0; i < ITERATIONS; i++) {
    let val = ns.namespaceGetVariable('testField50000', SType.REFERENCE);
  }
}

function testNamespaceSetString50(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  let string50 = new Array(50).fill('a').join('');
  for (let i = 0; i < ITERATIONS; i++) {
    ns.namespaceSetVariable('testField50', STValue.wrapString(string50), SType.REFERENCE);
  }
}

function testNamespaceSetString100(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  let string100 = new Array(100).fill('a').join('');
  for (let i = 0; i < ITERATIONS; i++) {
    ns.namespaceSetVariable('testField100', STValue.wrapString(string100), SType.REFERENCE);
  }
}

function testNamespaceSetString500(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  let string500 = new Array(500).fill('a').join('');
  for (let i = 0; i < ITERATIONS; i++) {
    ns.namespaceSetVariable('testField500', STValue.wrapString(string500), SType.REFERENCE);
  }
}

function testNamespaceSetString1000(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  let string1000 = new Array(1000).fill('a').join('');
  for (let i = 0; i < ITERATIONS; i++) {
    ns.namespaceSetVariable('testField1000', STValue.wrapString(string1000), SType.REFERENCE);
  }
}

function testNamespaceSetString5000(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  let string5000 = new Array(5000).fill('a').join('');
  for (let i = 0; i < ITERATIONS; i++) {
    ns.namespaceSetVariable('testField5000', STValue.wrapString(string5000), SType.REFERENCE);
  }
}

function testNamespaceSetString10000(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  let string10000 = new Array(10000).fill('a').join('');
  for (let i = 0; i < ITERATIONS; i++) {
    ns.namespaceSetVariable('testField10000', STValue.wrapString(string10000), SType.REFERENCE);
  }
}

function testNamespaceSetString50000(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  let string50000 = new Array(50000).fill('a').join('');
  for (let i = 0; i < ITERATIONS; i++) {
    ns.namespaceSetVariable('testField50000', STValue.wrapString(string50000), SType.REFERENCE);
  }
}

function testNamespaceGetNumber(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  for (let i = 0; i < ITERATIONS; i++) {
    let val = ns.namespaceGetVariable('testNumberField', SType.DOUBLE);
  }
}

function testNamespaceSetNumber(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  for (let i = 0; i < ITERATIONS; i++) {
    ns.namespaceSetVariable('testNumberField', STValue.wrapNumber(42.5), SType.DOUBLE);
  }
}

function testNamespaceGetBoolean(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  for (let i = 0; i < ITERATIONS; i++) {
    let val = ns.namespaceGetVariable('testBooleanField', SType.BOOLEAN);
  }
}

function testNamespaceSetBoolean(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  for (let i = 0; i < ITERATIONS; i++) {
    ns.namespaceSetVariable('testBooleanField', STValue.wrapBoolean(false), SType.BOOLEAN);
  }
}

// namespaceInvokeFunction tests
function testNamespaceInvokeFunctionNoArgs(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  let args = [];
  for (let i = 0; i < ITERATIONS; i++) {
    let result = ns.namespaceInvokeFunction('functionNoArgs', ':d', args);
  }
}

function testNamespaceInvokeFunctionOneArgNumber(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  let args = [STValue.wrapNumber(21)];
  for (let i = 0; i < ITERATIONS; i++) {
    let result = ns.namespaceInvokeFunction('functionOneArgNumber', 'd:d', args);
  }
}

function testNamespaceInvokeFunctionOneArgString(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  let args = [STValue.wrapString('hello')];
  for (let i = 0; i < ITERATIONS; i++) {
    let result = ns.namespaceInvokeFunction('functionOneArgString', 'C{std.core.String}:d', args);
  }
}

function testNamespaceInvokeFunctionFiveArgsNumber(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  let args = [STValue.wrapNumber(1), STValue.wrapNumber(2), STValue.wrapNumber(3), STValue.wrapNumber(4), STValue.wrapNumber(5)];
  for (let i = 0; i < ITERATIONS; i++) {
    let result = ns.namespaceInvokeFunction('functionFiveArgsNumber', 'ddddd:d', args);
  }
}

function testNamespaceInvokeFunctionFiveArgsString(): void {
  let ns = STValue.findNamespace('stvalue_workload.stvalue_workload');
  let args = [STValue.wrapString('a'), STValue.wrapString('ab'), STValue.wrapString('abc'),
              STValue.wrapString('abcd'), STValue.wrapString('abcde')];
  for (let i = 0; i < ITERATIONS; i++) {
    let result = ns.namespaceInvokeFunction('functionFiveArgsString', 'C{std.core.String}C{std.core.String}C{std.core.String}C{std.core.String}C{std.core.String}:d', args);
  }
}

// classInvokeStaticMethod tests
function testClassInvokeStaticMethodNoArgs(): void {
  let cls = STValue.findClass('stvalue_workload.TestStaticClass');
  let args = [];
  for (let i = 0; i < ITERATIONS; i++) {
    let result = cls.classInvokeStaticMethod('staticMethodNoArgs', ':d', args);
  }
}

function testClassInvokeStaticMethodOneArgNumber(): void {
  let cls = STValue.findClass('stvalue_workload.TestStaticClass');
  let args = [STValue.wrapNumber(21)];
  for (let i = 0; i < ITERATIONS; i++) {
    let result = cls.classInvokeStaticMethod('staticMethodOneArgNumber', 'd:d', args);
  }
}

function testClassInvokeStaticMethodOneArgString(): void {
  let cls = STValue.findClass('stvalue_workload.TestStaticClass');
  let args = [STValue.wrapString('hello')];
  for (let i = 0; i < ITERATIONS; i++) {
    let result = cls.classInvokeStaticMethod('staticMethodOneArgString', 'C{std.core.String}:d', args);
  }
}

function testClassInvokeStaticMethodFiveArgsNumber(): void {
  let cls = STValue.findClass('stvalue_workload.TestStaticClass');
  let args = [STValue.wrapNumber(1), STValue.wrapNumber(2), STValue.wrapNumber(3), STValue.wrapNumber(4), STValue.wrapNumber(5)];
  for (let i = 0; i < ITERATIONS; i++) {
    let result = cls.classInvokeStaticMethod('staticMethodFiveArgsNumber', 'ddddd:d', args);
  }
}

function testClassInvokeStaticMethodFiveArgsString(): void {
  let cls = STValue.findClass('stvalue_workload.TestStaticClass');
  let args = [STValue.wrapString('a'), STValue.wrapString('ab'), STValue.wrapString('abc'),
              STValue.wrapString('abcd'), STValue.wrapString('abcde')];
  for (let i = 0; i < ITERATIONS; i++) {
    let result = cls.classInvokeStaticMethod('staticMethodFiveArgsString', 'C{std.core.String}C{std.core.String}C{std.core.String}C{std.core.String}C{std.core.String}:d', args);
  }
}

// objectInvokeMethod tests
function testObjectInvokeMethodNoArgs(): void {
  let cls = STValue.findClass('stvalue_workload.TestInstanceClass');
  let obj = cls.classInstantiate(':', []);
  let args = [];
  for (let i = 0; i < ITERATIONS; i++) {
    let result = obj.objectInvokeMethod('methodNoArgs', ':d', args);
  }
}

function testObjectInvokeMethodOneArgNumber(): void {
  let cls = STValue.findClass('stvalue_workload.TestInstanceClass');
  let obj = cls.classInstantiate(':', []);
  let args = [STValue.wrapNumber(21)];
  for (let i = 0; i < ITERATIONS; i++) {
    let result = obj.objectInvokeMethod('methodOneArgNumber', 'd:d', args);
  }
}

function testObjectInvokeMethodOneArgString(): void {
  let cls = STValue.findClass('stvalue_workload.TestInstanceClass');
  let obj = cls.classInstantiate(':', []);
  let args = [STValue.wrapString('hello')];
  for (let i = 0; i < ITERATIONS; i++) {
    let result = obj.objectInvokeMethod('methodOneArgString', 'C{std.core.String}:d', args);
  }
}

function testObjectInvokeMethodFiveArgsNumber(): void {
  let cls = STValue.findClass('stvalue_workload.TestInstanceClass');
  let obj = cls.classInstantiate(':', []);
  let args = [STValue.wrapNumber(1), STValue.wrapNumber(2), STValue.wrapNumber(3), STValue.wrapNumber(4), STValue.wrapNumber(5)];
  for (let i = 0; i < ITERATIONS; i++) {
    let result = obj.objectInvokeMethod('methodFiveArgsNumber', 'ddddd:d', args);
  }
}

function testObjectInvokeMethodFiveArgsString(): void {
  let cls = STValue.findClass('stvalue_workload.TestInstanceClass');
  let obj = cls.classInstantiate(':', []);
  let args = [STValue.wrapString('a'), STValue.wrapString('ab'), STValue.wrapString('abc'),
              STValue.wrapString('abcd'), STValue.wrapString('abcde')];
  for (let i = 0; i < ITERATIONS; i++) {
    let result = obj.objectInvokeMethod('methodFiveArgsString', 'C{std.core.String}C{std.core.String}C{std.core.String}C{std.core.String}C{std.core.String}:d', args);
  }
}

// objectGet and objectSet tests
function testObjectGetString50(): void {
  let cls = STValue.findClass('stvalue_workload.TestFieldClass');
  let obj = cls.classInstantiate(':', []);
  for (let i = 0; i < ITERATIONS; i++) {
    let val = obj.objectGetProperty('string50', SType.REFERENCE);
  }
}

function testObjectGetString100(): void {
  let cls = STValue.findClass('stvalue_workload.TestFieldClass');
  let obj = cls.classInstantiate(':', []);
  for (let i = 0; i < ITERATIONS; i++) {
    let val = obj.objectGetProperty('string100', SType.REFERENCE);
  }
}

function testObjectGetString500(): void {
  let cls = STValue.findClass('stvalue_workload.TestFieldClass');
  let obj = cls.classInstantiate(':', []);
  for (let i = 0; i < ITERATIONS; i++) {
    let val = obj.objectGetProperty('string500', SType.REFERENCE);
  }
}

function testObjectGetString1000(): void {
  let cls = STValue.findClass('stvalue_workload.TestFieldClass');
  let obj = cls.classInstantiate(':', []);
  for (let i = 0; i < ITERATIONS; i++) {
    let val = obj.objectGetProperty('string1000', SType.REFERENCE);
  }
}

function testObjectGetString5000(): void {
  let cls = STValue.findClass('stvalue_workload.TestFieldClass');
  let obj = cls.classInstantiate(':', []);
  for (let i = 0; i < ITERATIONS; i++) {
    let val = obj.objectGetProperty('string5000', SType.REFERENCE);
  }
}

function testObjectGetString10000(): void {
  let cls = STValue.findClass('stvalue_workload.TestFieldClass');
  let obj = cls.classInstantiate(':', []);
  for (let i = 0; i < ITERATIONS; i++) {
    let val = obj.objectGetProperty('string10000', SType.REFERENCE);
  }
}

function testObjectGetString50000(): void {
  let cls = STValue.findClass('stvalue_workload.TestFieldClass');
  let obj = cls.classInstantiate(':', []);
  for (let i = 0; i < ITERATIONS; i++) {
    let val = obj.objectGetProperty('string50000', SType.REFERENCE);
  }
}

function testObjectSetString50(): void {
  let cls = STValue.findClass('stvalue_workload.TestFieldClass');
  let obj = cls.classInstantiate(':', []);
  let string50 = new Array(50).fill('a').join('');
  for (let i = 0; i < ITERATIONS; i++) {
    obj.objectSetProperty('string50', STValue.wrapString(string50), SType.REFERENCE);
  }
}

function testObjectSetString100(): void {
  let cls = STValue.findClass('stvalue_workload.TestFieldClass');
  let obj = cls.classInstantiate(':', []);
  let string100 = new Array(100).fill('a').join('');
  for (let i = 0; i < ITERATIONS; i++) {
    obj.objectSetProperty('string100', STValue.wrapString(string100), SType.REFERENCE);
  }
}

function testObjectSetString500(): void {
  let cls = STValue.findClass('stvalue_workload.TestFieldClass');
  let obj = cls.classInstantiate(':', []);
  let string500 = new Array(500).fill('a').join('');
  for (let i = 0; i < ITERATIONS; i++) {
    obj.objectSetProperty('string500', STValue.wrapString(string500), SType.REFERENCE);
  }
}

function testObjectSetString1000(): void {
  let cls = STValue.findClass('stvalue_workload.TestFieldClass');
  let obj = cls.classInstantiate(':', []);
  let string1000 = new Array(1000).fill('a').join('');
  for (let i = 0; i < ITERATIONS; i++) {
    obj.objectSetProperty('string1000', STValue.wrapString(string1000), SType.REFERENCE);
  }
}

function testObjectSetString5000(): void {
  let cls = STValue.findClass('stvalue_workload.TestFieldClass');
  let obj = cls.classInstantiate(':', []);
  let string5000 = new Array(5000).fill('a').join('');
  for (let i = 0; i < ITERATIONS; i++) {
    obj.objectSetProperty('string5000', STValue.wrapString(string5000), SType.REFERENCE);
  }
}

function testObjectSetString10000(): void {
  let cls = STValue.findClass('stvalue_workload.TestFieldClass');
  let obj = cls.classInstantiate(':', []);
  let string10000 = new Array(10000).fill('a').join('');
  for (let i = 0; i < ITERATIONS; i++) {
    obj.objectSetProperty('string10000', STValue.wrapString(string10000), SType.REFERENCE);
  }
}

function testObjectSetString50000(): void {
  let cls = STValue.findClass('stvalue_workload.TestFieldClass');
  let obj = cls.classInstantiate(':', []);
  let string50000 = new Array(50000).fill('a').join('');
  for (let i = 0; i < ITERATIONS; i++) {
    obj.objectSetProperty('string50000', STValue.wrapString(string50000), SType.REFERENCE);
  }
}

function testObjectGetNumber(): void {
  let cls = STValue.findClass('stvalue_workload.TestFieldClass');
  let obj = cls.classInstantiate(':', []);
  for (let i = 0; i < ITERATIONS; i++) {
    let val = obj.objectGetProperty('numberField', SType.DOUBLE);
  }
}

function testObjectSetNumber(): void {
  let cls = STValue.findClass('stvalue_workload.TestFieldClass');
  let obj = cls.classInstantiate(':', []);
  for (let i = 0; i < ITERATIONS; i++) {
    obj.objectSetProperty('numberField', STValue.wrapNumber(42.5), SType.DOUBLE);
  }
}

function testObjectGetBoolean(): void {
  let cls = STValue.findClass('stvalue_workload.TestFieldClass');
  let obj = cls.classInstantiate(':', []);
  for (let i = 0; i < ITERATIONS; i++) {
    let val = obj.objectGetProperty('booleanField', SType.BOOLEAN);
  }
}

function testObjectSetBoolean(): void {
  let cls = STValue.findClass('stvalue_workload.TestFieldClass');
  let obj = cls.classInstantiate(':', []);
  for (let i = 0; i < ITERATIONS; i++) {
    obj.objectSetProperty('booleanField', STValue.wrapBoolean(false), SType.BOOLEAN);
  }
}

function main(): void {
  testFindClassStdCore();
  testFindClassUserDefined();
  testFindNamespace();
  testFindClassMultiple();

  testClassInstantiateNoArgs();
  testClassInstantiateWithArgs();

  testNamespaceGetString50();
  testNamespaceGetString100();
  testNamespaceGetString500();
  testNamespaceGetString1000();
  testNamespaceGetString5000();
  testNamespaceGetString10000();
  testNamespaceGetString50000();

  testNamespaceSetString50();
  testNamespaceSetString100();
  testNamespaceSetString500();
  testNamespaceSetString1000();
  testNamespaceSetString5000();
  testNamespaceSetString10000();
  testNamespaceSetString50000();

  testNamespaceGetNumber();
  testNamespaceSetNumber();

  testNamespaceGetBoolean();
  testNamespaceSetBoolean();

  testNamespaceInvokeFunctionNoArgs();
  testNamespaceInvokeFunctionOneArgNumber();
  testNamespaceInvokeFunctionOneArgString();
  testNamespaceInvokeFunctionFiveArgsNumber();
  testNamespaceInvokeFunctionFiveArgsString();

  testClassInvokeStaticMethodNoArgs();
  testClassInvokeStaticMethodOneArgNumber();
  testClassInvokeStaticMethodOneArgString();
  testClassInvokeStaticMethodFiveArgsNumber();
  testClassInvokeStaticMethodFiveArgsString();

  testObjectInvokeMethodNoArgs();
  testObjectInvokeMethodOneArgNumber();
  testObjectInvokeMethodOneArgString();
  testObjectInvokeMethodFiveArgsNumber();
  testObjectInvokeMethodFiveArgsString();

  testObjectGetString50();
  testObjectGetString100();
  testObjectGetString500();
  testObjectGetString1000();
  testObjectGetString5000();
  testObjectGetString10000();
  testObjectGetString50000();

  testObjectSetString50();
  testObjectSetString100();
  testObjectSetString500();
  testObjectSetString1000();
  testObjectSetString5000();
  testObjectSetString10000();
  testObjectSetString50000();

  testObjectGetNumber();
  testObjectSetNumber();

  testObjectGetBoolean();
  testObjectSetBoolean();
}

main();
