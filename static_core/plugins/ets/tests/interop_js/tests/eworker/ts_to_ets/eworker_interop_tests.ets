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

let eaw = new EAWorker(true);

function testProcessInput() {
    let callback = ():boolean => {
        let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/eworker/ts_to_ets/eworker_helpers');
        let processInputDy = module.getProperty('processInput');
        let res1 = processInputDy.invoke(ESValue.wrapNumber(5)); //  2 * 5 = 10
        let res2 = processInputDy.invoke(ESValue.wrapString('hello')); // 'HELLO'
        let res3 = processInputDy.invoke(ESValue.wrapBoolean(true)); // false
        let res = (res1.toNumber() === 10) && (res2.toString() === 'HELLO') && (!res3.toBoolean());
        return res;
    };

    let res = eaw.run<boolean>(callback).Await() as boolean;
    assertTrue(res);
}

function testCreateArray() {
    let callback = ():boolean => {
        let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/eworker/ts_to_ets/eworker_helpers');
        let createArrayDy = module.getProperty('createArray');
        let res1 = createArrayDy.invoke(ESValue.wrapNumber(5), ESValue.wrapNumber(1));
        let res2 = createArrayDy.invoke(ESValue.wrapString('a'), ESValue.wrapNumber(1), ESValue.wrapNumber(4));
        let res3 = createArrayDy.invoke(ESValue.wrapBoolean(true), ESValue.wrapNumber(2));
        return (res1.getProperty(0).toNumber() === 5) &&
                    (res2.getProperty(0).toString() === 'a') &&
                    (res3.getProperty(0).toBoolean() === true);
    };
    let res = eaw.run<boolean>(callback).Await() as boolean;
    assertTrue(res);
}

function testGetUser() {
    let callback = ():boolean => {
        let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/eworker/ts_to_ets/eworker_helpers');
        let getUserDy = module.getProperty('getUser');
        // { id: 123, name: 'John Doe' }
        let res1 = getUserDy.invoke(ESValue.wrapNumber(123));
        // { id: 1, name: 'admin' }
        let res2 = getUserDy.invoke(ESValue.wrapString('admin'));
        return res1.getProperty('id').toNumber() === 123 &&
                    res2.getProperty('role').toString() === 'admin';
    };
    let res = eaw.run<boolean>(callback).Await() as boolean;
    assertTrue(res);
}

class NewTestClass {
  prop: number = 1;
}

function testUnion() {
  let result = false;
  try {
    let callback = () => {
        let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/eworker/ts_to_ets/eworker_helpers');
        let dynamicObj = module.getProperty('dynamicObj');
        let TestClass = module.getProperty("TestClass");
        dynamicObj.setProperty('unionProp', ESValue.wrapNumber(123));
        assertTrue(dynamicObj.getProperty('unionProp').toNumber() === 123);
        dynamicObj.setProperty('unionProp', ESValue.wrapString('123'));
        assertTrue(dynamicObj.getProperty('unionProp').toString() === '123');
        dynamicObj.setProperty('unionProp', ESValue.wrapBoolean(true));
        assertTrue(dynamicObj.getProperty('unionProp').toBoolean() === true);
        let testInstance = TestClass.instantiate();
        dynamicObj.setProperty('unionProp', testInstance);
        assertTrue(dynamicObj.getProperty('unionProp').getProperty('name').toString() === 'aaa');
        
        dynamicObj.setProperty('unionProp', ESValue.wrap(new NewTestClass()));
        assertTrue(dynamicObj.getProperty('unionProp').getProperty('prop').toNumber() === 1);
        result = true;
    }
    eaw.run<void>(callback).Await();
  } catch (error) {
    console.error(error.toString());
    assertFalse(result);
  }
  assertTrue(result);
}

function testBaseClass() {
    let callback = () => {
        let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/eworker/ts_to_ets/eworker_helpers')
        let obj = module.getProperty('Base').instantiate().invokeMethod('getCurrent')
        assertTrue(obj.typeOf() === 'object')
        assertTrue(obj.getProperty('baseVal').toNumber() === 1)
    }
    eaw.run<void>(callback).Await();
}

function testChildClass() {
    let callback = () => {
        let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/eworker/ts_to_ets/eworker_helpers')
        let obj = module.getProperty('Child').instantiate().invokeMethod('getSuper')
        assertTrue(obj.typeOf() === 'object')
        assertTrue(obj.getProperty('baseVal').toNumber() === 1)
        assertTrue(obj.getProperty('childVal').toNumber() === 2)
    }
    eaw.run<void>(callback).Await();
}

function testBaseFunc() {
    let callback = () => {
        let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/eworker/ts_to_ets/eworker_helpers')
        let obj = module.getProperty('baseFunc').invoke()
        assertTrue(ESValue.areStrictlyEqual(obj, ESValue.Undefined))
    }
    eaw.run<void>(callback).Await();
}

function testChildFunc() {
    let callback = () => {
        let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/eworker/ts_to_ets/eworker_helpers')
        let obj = module.getProperty('childFunc').invoke()
        assertTrue(ESValue.areStrictlyEqual(obj, ESValue.Undefined))
    }
    eaw.run<void>(callback).Await();
}

function testFooBaseObjInvoke() {
    let callback = () => {
        let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/eworker/ts_to_ets/eworker_helpers')
        let fooBaseObj = module.getProperty('fooBaseObj')
        let result = fooBaseObj.getProperty('foo').invoke()
        assertTrue(ESValue.areStrictlyEqual(result, ESValue.Undefined))
    }
    eaw.run<void>(callback).Await();
}

function testFooBaseObjInvokeMethod() {
    let callback = () => {
        let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/eworker/ts_to_ets/eworker_helpers')
        let fooBaseObj = module.getProperty('fooBaseObj')
        let result = fooBaseObj.invokeMethod('fooFunc')
        assertTrue(ESValue.areStrictlyEqual(result, fooBaseObj))
    }
    eaw.run<void>(callback).Await();
}

function testFooChildObjInvoke() {
    let callback = () => {
        let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/eworker/ts_to_ets/eworker_helpers')
        let fooChildObj = module.getProperty('fooChildObj')
        let result = fooChildObj.getProperty('foo').invoke()
        assertTrue(ESValue.areStrictlyEqual(result, ESValue.Undefined))
    }
    eaw.run<void>(callback).Await();
}

function testFooChildObjInvokeMethod() {
    let callback = () => {
        let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/eworker/ts_to_ets/eworker_helpers')
        let fooChildObj = module.getProperty('fooChildObj')
        let result = fooChildObj.invokeMethod('fooFunc')
        assertTrue(ESValue.areStrictlyEqual(result, fooChildObj))
    }
    eaw.run<void>(callback).Await();
}

function testArrowFuncInvoke() {
    let callback = () => {
        let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/eworker/ts_to_ets/eworker_helpers');
        let result = module.getProperty('arrowFunc').invoke()
        assertTrue(ESValue.areStrictlyEqual(result, ESValue.Undefined))
    }
    eaw.run<void>(callback).Await();
}

function testOptionalParams() {
    let callback = () => {
        let module = ESValue.load("../../plugins/ets/tests/interop_js/tests/eworker/ts_to_ets/eworker_helpers");
        assertTrue(module.getProperty("foo1").invoke(ESValue.wrap(1), ESValue.wrap("str")).toBoolean());
        assertTrue(module.getProperty("foo2").invoke(ESValue.wrap(1), ESValue.wrap("str"), ESValue.wrap(true)).toBoolean());
        assertTrue(module.getProperty("foo3").invoke(ESValue.wrap(1), ESValue.wrap("str"), ESValue.wrap(true), ESValue.wrap(2)).toBoolean());

        assertTrue(module.getProperty("fun1").invoke(ESValue.wrap(1), ESValue.wrap("str")).toBoolean());
        assertTrue(module.getProperty("fun2").invoke(ESValue.wrap(1), ESValue.wrap("str"), ESValue.wrap(false)).toBoolean());
        assertTrue(module.getProperty("fun3").invoke(ESValue.wrap(1), ESValue.wrap("str"), ESValue.wrap(false), ESValue.wrap(2)).toBoolean());
    }
    eaw.run<void>(callback).Await();
}


const ErrMsg: string = 'This is a Error';
const ErrorMessage: string = 'Panda: throw new error in ets!';
const CustomErrorMessage: string = 'Panda: Something went wrong!';
const CustomErrorCode: number = 1001;

function testError() {
    let callback = () => {
        let module = ESValue.load("../../plugins/ets/tests/interop_js/tests/eworker/ts_to_ets/eworker_helpers");
        let err = module.getProperty("err");
        let res: boolean;
        res = (err.getProperty("message").toString() == ErrMsg);
        return res;
    }
    let res = eaw.run<boolean>(callback).Await() as boolean;
    assertTrue(res);
}

function testThrowError() {
    let callback = () => {
    let module = ESValue.load("../../plugins/ets/tests/interop_js/tests/eworker/ts_to_ets/eworker_helpers");
    let res: boolean;
    try {
        module.getProperty("ErrorFunc").invoke(ESValue.wrapString(ErrorMessage));
        res = false;
        } catch (error: Error) {
            res = (error instanceof Error) && (error.message == ErrorMessage);
        }
        return res;
    }
    let res = eaw.run<boolean>(callback).Await() as boolean;
    assertTrue(res);
}

function testCustomThrowError() {
    let callback = () => {
        let module = ESValue.load("../../plugins/ets/tests/interop_js/tests/eworker/ts_to_ets/eworker_helpers");
        let res: boolean;
        try {
            module.getProperty("CustomErrorFunc").invoke(ESValue.wrapString(CustomErrorMessage), ESValue.wrapNumber(CustomErrorCode));
            res = false;
        } catch (e: Error) {
            let errObj: ESValue = (e as ESError).getValue()
            let msg: ESValue = errObj.getProperty('message')
            let code: ESValue = errObj.getProperty('code')
            res = (e instanceof Error) && (msg.toString() == CustomErrorMessage) &&
                (code.toNumber() == CustomErrorCode);
        }
        return res;
    }
    let res = eaw.run<boolean>(callback).Await() as boolean;
    assertTrue(res);
}

function testEAWorkerNestedExecute() {
    let callback = ():boolean => {
        let innerWorker = new EAWorker(true);
        let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/eworker/ts_to_ets/eworker_helpers');
        let processInputDy = module.getProperty('processInput');
        let res1 = processInputDy.invoke(ESValue.wrapNumber(5)); //  2 * 5 = 10
        let res2 = processInputDy.invoke(ESValue.wrapString('hello')); // 'HELLO'
        let res3 = processInputDy.invoke(ESValue.wrapBoolean(true)); // false
        let res = (res1.toNumber() === 10) && (res2.toString() === 'HELLO') && (!res3.toBoolean());

        let callbackInner = ():boolean => {
            let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/eworker/ts_to_ets/eworker_helpers');
            let processInputDy = module.getProperty('processInput');
            let res1 = processInputDy.invoke(ESValue.wrapNumber(5)); //  2 * 5 = 10
            let res2 = processInputDy.invoke(ESValue.wrapString('hello')); // 'HELLO'
            let res3 = processInputDy.invoke(ESValue.wrapBoolean(true)); // false
            return res1.toNumber() === 10 && res2.toString() === 'HELLO' && !res3.toBoolean();
        }
        let resInner = innerWorker.run<boolean>(callbackInner).Await() as boolean;
        innerWorker.join();
        assertTrue(resInner);
        return res && resInner;
    };
    let res = eaw.run<boolean>(callback).Await() as boolean;
    assertEQ(res, true);
}

function testEAWorkerInteropLoop() {
    let callback = () => {
        let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/eworker/ts_to_ets/eworker_helpers');
        let processInputDy = module.getProperty('processInput');
        for (let i = 0; i < 1000; i++) {
            let res1 = processInputDy.invoke(ESValue.wrapNumber(5)); //  2 * 5 = 10
            let res2 = processInputDy.invoke(ESValue.wrapString('hello')); // 'HELLO'
            let res3 = processInputDy.invoke(ESValue.wrapBoolean(true)); // false
            assertEQ(res1.toNumber(), 10);
            assertEQ(res2.toString(), 'HELLO');
            assertEQ(res3.toBoolean(), false);
        }
    }
    let res = eaw.run<boolean>(callback).Await() as boolean;
    assertEQ(res, true);
}

function eworkerInteropTest() {
    let eworkerInteropSuite = new ArkTestsuite("eworker.InteropTest");
    eworkerInteropSuite.addTest("testProcessInput", testProcessInput);
    eworkerInteropSuite.addTest("testCreateArray", testCreateArray);
    eworkerInteropSuite.addTest("testGetUser", testGetUser);
    eworkerInteropSuite.addTest("testUnion", testUnion);
    eworkerInteropSuite.addTest("testBaseClass", testBaseClass);
    eworkerInteropSuite.addTest("testChildClass", testChildClass);
    eworkerInteropSuite.addTest("testBaseFunc", testBaseFunc);
    eworkerInteropSuite.addTest("testChildFunc", testChildFunc);
    eworkerInteropSuite.addTest("testFooBaseObjInvoke", testFooBaseObjInvoke);
    eworkerInteropSuite.addTest("testFooBaseObjInvokeMethod", testFooBaseObjInvokeMethod);
    eworkerInteropSuite.addTest("testFooChildObjInvoke", testFooChildObjInvoke);
    eworkerInteropSuite.addTest("testFooChildObjInvokeMethod", testFooChildObjInvokeMethod);
    eworkerInteropSuite.addTest("testArrowFuncInvoke", testArrowFuncInvoke);
    eworkerInteropSuite.addTest("testOptionalParams", testOptionalParams);
    eworkerInteropSuite.addTest("testError", testError);
    eworkerInteropSuite.addTest("testThrowError", testThrowError);
    eworkerInteropSuite.addTest("testCustomThrowError", testCustomThrowError);
    eworkerInteropSuite.addTest("testEAWorkerNestedExecute", testEAWorkerNestedExecute);
    eworkerInteropSuite.addTest("testEAWorkerInteropLoop", testEAWorkerInteropLoop);
    eworkerInteropSuite.run();
    eaw.join();
    return true;
}


