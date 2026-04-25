/**
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
async function testProcessInput(): Promise<void> {
    let callback = ():boolean => {
        let module = ESValue.load('concurrency_helpers');
        let processInputDy = module.getProperty('processInput');
        let res1 = processInputDy.invoke(ESValue.wrapNumber(5)); //  2 * 5 = 10
        let res2 = processInputDy.invoke(ESValue.wrapString('hello')); // 'HELLO'
        let res3 = processInputDy.invoke(ESValue.wrapBoolean(true)); // false
        let res = (res1.toNumber() === 10) && (res2.toString() === 'HELLO') && (!res3.toBoolean());
        return res;
    };
    let res = (await taskpool.execute(callback)) as boolean;
    arktest.assertEQ(res, true);
}

async function testCreateArray(): Promise<void> {
    let callback = ():boolean => {
        let module = ESValue.load('concurrency_helpers');
        let createArrayDy = module.getProperty('createArray');
        // [5]
        let res1 = createArrayDy.invoke(ESValue.wrapNumber(5), ESValue.wrapNumber(1));
        // ['a', 1, 'a', 1]
        let res2 = createArrayDy.invoke(ESValue.wrapString('a'), ESValue.wrapNumber(1), ESValue.wrapNumber(4));
        // [true, true]
        let res3 = createArrayDy.invoke(ESValue.wrapBoolean(true), ESValue.wrapNumber(2));
        return (res1.getProperty(0).toNumber() === 5) &&
                    (res2.getProperty(0).toString() === 'a') &&
                    (res3.getProperty(0).toBoolean() === true);
    };
    let res = (await taskpool.execute(callback)) as boolean;
    arktest.assertEQ(res, true);
}

async function testGetUser(): Promise<void> {
    let callback = ():boolean => {
        let module = ESValue.load('concurrency_helpers');
        let getUserDy = module.getProperty('getUser');
        // { id: 123, name: 'John Doe' }
        let res1 = getUserDy.invoke(ESValue.wrapNumber(123));
        // { id: 1, name: 'admin' }
        let res2 = getUserDy.invoke(ESValue.wrapString('admin'));
        return res1.getProperty('id').toNumber() === 123 &&
                    res2.getProperty('role').toString() === 'admin';
    };
    let res = (await taskpool.execute(callback)) as boolean;
    arktest.assertEQ(res, true);
}

class NewTestClass {
  prop: number = 1;
}

async function testUnion(): Promise<void> {
  let result = false;
  try {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let dynamicObj = module.getProperty('dynamicObj');
        let TestClass = module.getProperty('TestClass');
        dynamicObj.setProperty('unionProp', ESValue.wrapNumber(123));
        arktest.assertTrue(dynamicObj.getProperty('unionProp').toNumber() === 123);
        dynamicObj.setProperty('unionProp', ESValue.wrapString('123'));
        arktest.assertTrue(dynamicObj.getProperty('unionProp').toString() === '123');
        dynamicObj.setProperty('unionProp', ESValue.wrapBoolean(true));
        arktest.assertTrue(dynamicObj.getProperty('unionProp').toBoolean() === true);
        let testInstance = TestClass.instantiate();
        dynamicObj.setProperty('unionProp', testInstance);
        arktest.assertTrue(dynamicObj.getProperty('unionProp').getProperty('name').toString() === 'aaa');
        
        dynamicObj.setProperty('unionProp', ESValue.wrap(new NewTestClass()));
        arktest.assertTrue(dynamicObj.getProperty('unionProp').getProperty('prop').toNumber() === 1);
        result = true;
    }
    await taskpool.execute(callback);
  } catch (error) {
    console.error(error.toString());
    arktest.assertFalse(result);
  }
  arktest.assertEQ(result, true);
}

async function testBaseClass(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let obj = module.getProperty('Base').instantiate().invokeMethod('getCurrent')
        arktest.assertTrue(obj.typeOf() === 'object')
        arktest.assertTrue(obj.getProperty('baseVal').toNumber() === 1)
    }
    await taskpool.execute(callback);
}

async function testChildClass(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let obj = module.getProperty('Child').instantiate().invokeMethod('getSuper')
        arktest.assertTrue(obj.typeOf() === 'object')
        arktest.assertTrue(obj.getProperty('baseVal').toNumber() === 1)
        arktest.assertTrue(obj.getProperty('childVal').toNumber() === 2)
    }
    await taskpool.execute(callback);
}

async function testBaseFunc(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let obj = module.getProperty('baseFunc').invoke()
        arktest.assertTrue(ESValue.areStrictlyEqual(obj, ESValue.Undefined))
    }
    await taskpool.execute(callback);
}

async function testChildFunc(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let obj = module.getProperty('childFunc').invoke()
        arktest.assertTrue(ESValue.areStrictlyEqual(obj, ESValue.Undefined))
    }
    await taskpool.execute(callback);
}

async function testFooBaseObjInvoke(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let fooBaseObj = module.getProperty('fooBaseObj')
        let result = fooBaseObj.getProperty('foo').invoke()
        arktest.assertTrue(ESValue.areStrictlyEqual(result, ESValue.Undefined))
    }
    await taskpool.execute(callback);
}

async function testFooBaseObjInvokeMethod(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let fooBaseObj = module.getProperty('fooBaseObj')
        let result = fooBaseObj.invokeMethod('fooFunc')
        arktest.assertTrue(ESValue.areStrictlyEqual(result, fooBaseObj))
    }
    await taskpool.execute(callback);
}

async function testFooChildObjInvoke(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let fooChildObj = module.getProperty('fooChildObj')
        let result = fooChildObj.getProperty('foo').invoke()
        arktest.assertTrue(ESValue.areStrictlyEqual(result, ESValue.Undefined))
    }
    await taskpool.execute(callback);
}

async function testFooChildObjInvokeMethod(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let fooChildObj = module.getProperty('fooChildObj')
        let result = fooChildObj.invokeMethod('fooFunc')
        arktest.assertTrue(ESValue.areStrictlyEqual(result, fooChildObj))
    }
    await taskpool.execute(callback);
}

async function testArrowFuncInvoke(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let result = module.getProperty('arrowFunc').invoke()
        arktest.assertTrue(ESValue.areStrictlyEqual(result, ESValue.Undefined))
    }
    await taskpool.execute(callback);
}

async function testOptionalParams(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        arktest.assertTrue(module.getProperty('foo1').invoke(ESValue.wrap(1), ESValue.wrap('str')).toBoolean());
        arktest.assertTrue(module.getProperty('foo2').invoke(ESValue.wrap(1), ESValue.wrap('str'), ESValue.wrap(true)).toBoolean());
        arktest.assertTrue(module.getProperty('foo3').invoke(ESValue.wrap(1), ESValue.wrap('str'), ESValue.wrap(true), ESValue.wrap(2)).toBoolean());

        arktest.assertTrue(module.getProperty('fun1').invoke(ESValue.wrap(1), ESValue.wrap('str')).toBoolean());
        arktest.assertTrue(module.getProperty('fun2').invoke(ESValue.wrap(1), ESValue.wrap('str'), ESValue.wrap(false)).toBoolean());
        arktest.assertTrue(module.getProperty('fun3').invoke(ESValue.wrap(1), ESValue.wrap('str'), ESValue.wrap(false), ESValue.wrap(2)).toBoolean());
    }
    await taskpool.execute(callback);
}


const ErrMsg: string = 'This is a Error';
const ErrorMessage: string = 'Panda: throw new error in ets!';
const CustomErrorMessage: string = 'Panda: Something went wrong!';
const CustomErrorCode: number = 1001;

async function testError(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let err = module.getProperty('err');
        let res: boolean;
        res = (err.getProperty('message').toString() == ErrMsg);
        return res;
    }
    let res = (await taskpool.execute(callback)) as boolean;
    arktest.assertEQ(res, true);
}

async function testThrowError(): Promise<void> {
    let callback = () => {
    let module = ESValue.load('concurrency_helpers');
    let res: boolean;
    try {
        module.getProperty('ErrorFunc').invoke(ESValue.wrapString(ErrorMessage));
        res = false;
        } catch (error) {
            res = (error instanceof Error) && (error.message == ErrorMessage);
        }
        return res;
    }
    let res = (await taskpool.execute(callback)) as boolean;
    arktest.assertEQ(res, true);
}

async function testCustomThrowError(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let res: boolean;
        try {
            module.getProperty('CustomErrorFunc').invoke(ESValue.wrapString(CustomErrorMessage), ESValue.wrapNumber(CustomErrorCode));
            res = false;
    } catch (e) {
        res = (e instanceof Error) && (e.message == CustomErrorMessage) &&
              (e.code == CustomErrorCode);
        }
        return res;
    }
    let res = (await taskpool.execute(callback)) as boolean;
    arktest.assertEQ(res, true);
}

async function testTaskpoolNestedExecute(): Promise<void> {
    let callback = ():boolean => {
        let module = ESValue.load('concurrency_helpers');
        let processInputDy = module.getProperty('processInput');
        let res1 = processInputDy.invoke(ESValue.wrapNumber(5)); //  2 * 5 = 10
        let res2 = processInputDy.invoke(ESValue.wrapString('hello')); // 'HELLO'
        let res3 = processInputDy.invoke(ESValue.wrapBoolean(true)); // false
        let res = (res1.toNumber() === 10) && (res2.toString() === 'HELLO') && (!res3.toBoolean());

        let callbackInner = ():boolean => {
            let module = ESValue.load('concurrency_helpers');
            let processInputDy = module.getProperty('processInput');
            let res1 = processInputDy.invoke(ESValue.wrapNumber(5)); //  2 * 5 = 10
            let res2 = processInputDy.invoke(ESValue.wrapString('hello')); // 'HELLO'
            let res3 = processInputDy.invoke(ESValue.wrapBoolean(true)); // false
            return res1.toNumber() === 10 && res2.toString() === 'HELLO' && !res3.toBoolean();
        }
        let resInner = (taskpool.execute(callbackInner).awaitSync()) as boolean;
        arktest.assertTrue(resInner);
        return res && resInner;
    };
    let res = (await taskpool.execute(callback)) as boolean;
    arktest.assertEQ(res, true);
}

async function testTaskpoolInteropLoop(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let processInputDy = module.getProperty('processInput');
        for (let i = 0; i < 100; i++) {
            let res1 = processInputDy.invoke(ESValue.wrapNumber(5)); //  2 * 5 = 10
            let res2 = processInputDy.invoke(ESValue.wrapString('hello')); // 'HELLO'
            let res3 = processInputDy.invoke(ESValue.wrapBoolean(true)); // false
            arktest.assertEQ(res1.toNumber(), 10);
            arktest.assertEQ(res2.toString(), 'HELLO');
            arktest.assertEQ(res3.toBoolean(), false);
        }
    }
    await taskpool.execute(callback);
}

function testSerializeFunc(serializeTestObj: ESValue) {
    let value1 = serializeTestObj.getProperty('value1').toString();
    let value2 = serializeTestObj.getProperty('value2').toNumber();
    let valueArray = serializeTestObj.getProperty('valueArray').getProperty(0).toString();
    arktest.assertEQ(value1, 'SerializeTest');
    arktest.assertEQ(value2, 1);
    arktest.assertEQ(valueArray, '1');
    return true;
}

async function testTaskpoolSerialize(): Promise<void> {
    let module = ESValue.load('concurrency_helpers');
    let serializeTestObj = module.getProperty('serializeTestObj');
    let res = (await taskpool.execute(testSerializeFunc, serializeTestObj)) as boolean;
    arktest.assertEQ(res, true);
}

async function testTaskpoolSerializeClassInstantiate(): Promise<void> {
    let module = ESValue.load('concurrency_helpers');
    let SerializeTest = module.getProperty('SerializeTest');
    let serializeTestObj = SerializeTest.instantiate('SerializeTest', 1, ['1', '2', '3']);
    let res = (await taskpool.execute(testSerializeFunc, serializeTestObj)) as boolean;
    arktest.assertEQ(res, true);
}

async function testTaskpoolSerializeUndefined(): Promise<void> {
    let module = ESValue.load('concurrency_helpers');
    let serializeObj = module.getProperty('serializeUndefined');
    let callback = (value: ESValue) => {
        arktest.assertTrue(value.isUndefined());
    }
    await taskpool.execute(callback, serializeObj);
}

async function testTaskpoolSerializeNull(): Promise<void> {
    let module = ESValue.load('concurrency_helpers');
    let serializeObj = module.getProperty('serializeNull');
    let callback = (value: ESValue) => {
        arktest.assertTrue(value.isNull());
    }
    await taskpool.execute(callback, serializeObj);
}

function taskpoolInteropTest() {
    let taskpoolInteropSuite = new arktest.ArkTestsuite('taskpool.InteropTest');
    taskpoolInteropSuite.addAsyncTest('testProcessInput', testProcessInput);
    taskpoolInteropSuite.addAsyncTest('testCreateArray', testCreateArray);
    taskpoolInteropSuite.addAsyncTest('testGetUser', testGetUser);
    // Flaky fail: #30965
    // taskpoolInteropSuite.addAsyncTest('testUnion', testUnion);
    taskpoolInteropSuite.addAsyncTest('testBaseClass', testBaseClass);
    taskpoolInteropSuite.addAsyncTest('testChildClass', testChildClass);
    taskpoolInteropSuite.addAsyncTest('testBaseFunc', testBaseFunc);
    taskpoolInteropSuite.addAsyncTest('testChildFunc', testChildFunc);
    taskpoolInteropSuite.addAsyncTest('testFooBaseObjInvoke', testFooBaseObjInvoke);
    taskpoolInteropSuite.addAsyncTest('testFooBaseObjInvokeMethod', testFooBaseObjInvokeMethod);
    taskpoolInteropSuite.addAsyncTest('testFooChildObjInvoke', testFooChildObjInvoke);
    taskpoolInteropSuite.addAsyncTest('testFooChildObjInvokeMethod', testFooChildObjInvokeMethod);
    taskpoolInteropSuite.addAsyncTest('testArrowFuncInvoke', testArrowFuncInvoke);
    taskpoolInteropSuite.addAsyncTest('testOptionalParams', testOptionalParams);
    taskpoolInteropSuite.addAsyncTest('testError', testError);
    taskpoolInteropSuite.addAsyncTest('testThrowError', testThrowError);
    taskpoolInteropSuite.addAsyncTest('testCustomThrowError', testCustomThrowError);
    taskpoolInteropSuite.addAsyncTest('testTaskpoolNestedExecute', testTaskpoolNestedExecute);
    taskpoolInteropSuite.addAsyncTest('testTaskpoolInteropLoop', testTaskpoolInteropLoop);
    taskpoolInteropSuite.addAsyncTest('testTaskpoolSerialize', testTaskpoolSerialize);
    taskpoolInteropSuite.addAsyncTest('testTaskpoolSerializeClassInstantiate', testTaskpoolSerializeClassInstantiate);
    taskpoolInteropSuite.addAsyncTest('testTaskpoolSerializeUndefined', testTaskpoolSerializeUndefined);
    taskpoolInteropSuite.addAsyncTest('testTaskpoolSerializeNull', testTaskpoolSerializeNull);
    const res = taskpoolInteropSuite.run();
    CoroutineExtras.stopTaskpool();
    return res == 0;
}
