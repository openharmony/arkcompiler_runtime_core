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

let eaw = new EAWorker(true);

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

    let res = await eaw.run<boolean>(callback) as boolean;
    arktest.assertTrue(res);
}

async function testCreateArray(): Promise<void> {
    let callback = ():boolean => {
        let module = ESValue.load('concurrency_helpers');
        let createArrayDy = module.getProperty('createArray');
        let res1 = createArrayDy.invoke(ESValue.wrapNumber(5), ESValue.wrapNumber(1));
        let res2 = createArrayDy.invoke(ESValue.wrapString('a'), ESValue.wrapNumber(1), ESValue.wrapNumber(4));
        let res3 = createArrayDy.invoke(ESValue.wrapBoolean(true), ESValue.wrapNumber(2));
        return (res1.getProperty(0).toNumber() === 5) &&
                    (res2.getProperty(0).toString() === 'a') &&
                    (res3.getProperty(0).toBoolean() === true);
    };
    let res = await eaw.run<boolean>(callback) as boolean;
    arktest.assertTrue(res);
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
    let res = await eaw.run<boolean>(callback) as boolean;
    arktest.assertTrue(res);
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
        let TestClass = module.getProperty("TestClass");
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
    await eaw.run<void>(callback);
  } catch (error) {
    console.error(error.toString());
    arktest.assertFalse(result);
  }
  arktest.assertTrue(result);
}

async function testBaseClass(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers')
        let obj = module.getProperty('Base').instantiate().invokeMethod('getCurrent')
        arktest.assertTrue(obj.typeOf() === 'object')
        arktest.assertTrue(obj.getProperty('baseVal').toNumber() === 1)
    }
    await eaw.run<void>(callback);
}

async function testChildClass(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers')
        let obj = module.getProperty('Child').instantiate().invokeMethod('getSuper')
        arktest.assertTrue(obj.typeOf() === 'object')
        arktest.assertTrue(obj.getProperty('baseVal').toNumber() === 1)
        arktest.assertTrue(obj.getProperty('childVal').toNumber() === 2)
    }
    await eaw.run<void>(callback);
}

async function testBaseFunc(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers')
        let obj = module.getProperty('baseFunc').invoke()
        arktest.assertTrue(ESValue.areStrictlyEqual(obj, ESValue.Undefined))
    }
    await eaw.run<void>(callback);
}

async function testChildFunc(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let obj = module.getProperty('childFunc').invoke()
        arktest.assertTrue(ESValue.areStrictlyEqual(obj, ESValue.Undefined))
    }
    await eaw.run<void>(callback);
}

async function testFooBaseObjInvoke(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let fooBaseObj = module.getProperty('fooBaseObj')
        let result = fooBaseObj.getProperty('foo').invoke()
        arktest.assertTrue(ESValue.areStrictlyEqual(result, ESValue.Undefined))
    }
    await eaw.run<void>(callback);
}

async function testFooBaseObjInvokeMethod(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let fooBaseObj = module.getProperty('fooBaseObj')
        let result = fooBaseObj.invokeMethod('fooFunc')
        arktest.assertTrue(ESValue.areStrictlyEqual(result, fooBaseObj))
    }
    await eaw.run<void>(callback);
}

async function testFooChildObjInvoke(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let fooChildObj = module.getProperty('fooChildObj')
        let result = fooChildObj.getProperty('foo').invoke()
        arktest.assertTrue(ESValue.areStrictlyEqual(result, ESValue.Undefined))
    }
    await eaw.run<void>(callback);
}

async function testFooChildObjInvokeMethod(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let fooChildObj = module.getProperty('fooChildObj')
        let result = fooChildObj.invokeMethod('fooFunc')
        arktest.assertTrue(ESValue.areStrictlyEqual(result, fooChildObj))
    }
    await eaw.run<void>(callback);
}

async function testArrowFuncInvoke(): Promise<void> {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let result = module.getProperty('arrowFunc').invoke()
        arktest.assertTrue(ESValue.areStrictlyEqual(result, ESValue.Undefined))
    }
    await eaw.run<void>(callback);
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
    await eaw.run<void>(callback);
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
    let res = await eaw.run<boolean>(callback) as boolean;
    arktest.assertTrue(res);
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
    let res = await eaw.run<boolean>(callback) as boolean;
    arktest.assertTrue(res);
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
    let res = await eaw.run<boolean>(callback) as boolean;
    arktest.assertTrue(res);
}

async function testEAWorkerNestedExecute(): Promise<void> {
    let callback = async (): Promise<boolean> => {
        let innerWorker = new EAWorker(true);
        innerWorker.start();
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
        let resInner = await innerWorker.run<boolean>(callbackInner) as boolean;
        innerWorker.quit();
        arktest.assertTrue(resInner);
        return res && resInner;
    };
    let res = await eaw.run<boolean>(callback) as boolean;
    arktest.assertEQ(res, true);
}

async function testEAWorkerInteropLoop(): Promise<void> {
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
        return true;
    }
    let res = await eaw.run<boolean>(callback) as boolean;
    arktest.assertEQ(res, true);
}

function testFunc(serializeTestObj: ESValue) {
    let value1 = serializeTestObj.getProperty('value1').toString();
    let value2 = serializeTestObj.getProperty('value2').toNumber();
    let valueArray = serializeTestObj.getProperty('valueArray').getProperty(0).toString();
    arktest.assertEQ(value1, 'SerializeTest');
    arktest.assertEQ(value2, 1);
    arktest.assertEQ(valueArray, '1');
    return true;
}

async function testEAWorkerSerialize(): Promise<void> {
    let module = ESValue.load('concurrency_helpers');
    let serializeTestObj = module.getProperty('serializeTestObj');
    let res = await eaw.run<boolean>(testFunc, serializeTestObj) as boolean;
    arktest.assertEQ(res, true);
}

async function testEAWorkerSerializeClassInstantiate(): Promise<void> {
    let module = ESValue.load('concurrency_helpers');
    let SerializeTest = module.getProperty('SerializeTest');
    let serializeTestObj = SerializeTest.instantiate('SerializeTest', 1, ['1', '2', '3']);
    let res = await eaw.run<boolean>(testFunc, serializeTestObj) as boolean;
    arktest.assertEQ(res, true);
}

async function testEAWorkerSerializeUndefined(): Promise<void> {
    let module = ESValue.load('concurrency_helpers');
    let serializeObj = module.getProperty('serializeUndefined');
    let callback = (value: ESValue) => {
        return value.isUndefined();
    }
    let res = await eaw.run<boolean>(callback, serializeObj) as boolean;
    arktest.assertEQ(res, true);
}

async function testEAWorkerSerializeNull(): Promise<void> {
    let module = ESValue.load('concurrency_helpers');
    let serializeObj = module.getProperty('serializeNull');
    let callback = (value: ESValue) => {
        return value.isNull();
    }
    let res = await eaw.run<boolean>(callback, serializeObj) as boolean;
    arktest.assertEQ(res, true);
}

function testEAWorkerMessageCancellation() {
    let handler = new concurrency.MessageHandler((msg: concurrency.Message)=>{
        if (msg.getObject() != undefined) {
            console.log("msg.getObject() " + msg.getObject());
        }
        if (msg.getCallback() != undefined) {
            console.log("callback");
            let callback = msg.getCallback() as () => void;
            callback();
        }
    }, eaw);

    let msg1 = new concurrency.Message(1, handler);
    let msg2 = new concurrency.Message(2, "msg2", handler);
    let cb = ()=>{
        arktest.assertEQ(handler.removeMessages(1), true);
        arktest.assertEQ(handler.removeMessages(2, "msg2"), true);
        arktest.assertEQ(handler.hasMessages(1), false);
        arktest.assertEQ(handler.hasMessages(2, "msg2"), false);
    }
    let msg3 = new concurrency.Message(cb, handler);

    arktest.assertEQ(handler.sendMessage(msg3), true);
    arktest.assertEQ(handler.sendMessage(msg2), true);
    arktest.assertEQ(handler.sendMessage(msg1), true);
}
class Event {
    public Wait() {
        while (this.flag.get() != true) {
            Coroutine.Schedule();
        }
    }

    public Fire() {
        this.flag.set(true);
    }

    private flag = new AtomicFlag(false);
};

type Callback = () => void;
enum MessageType {
    EMPTY,
    STRING,
    NUMBER,
    BOOLEAN,
    ARRAY,
}
function removeMessageHandlerTest() {
    let waiter = new Event();
    let handler: concurrency.MessageHandler | undefined;
    let restExecuted = false;
    handler = new concurrency.MessageHandler((msg: concurrency.Message)=>{
        if (msg.getWhat() == 1) {
            restExecuted = true;
        }
        if (msg.getCallback() != undefined) {
            let callback = msg.getCallback() as Callback;
            callback();
        }
    }, eaw);

    let msg = new concurrency.Message(1, handler!);
    let callback = () => {
        handler?.removeMessages(1);
        waiter.Fire();
    }
    let msg2 = new concurrency.Message(callback, handler!);

    handler?.sendMessage(msg2);
    handler?.sendMessage(msg);

    waiter.Wait();
    arktest.assertEQ(restExecuted, false);
}

function testEAWorkerCommmunicateWithMessage()
{
    let eaw2 = new EAWorker();
    let waiter = new Event();
    eaw2.start();

    let eaw1Tag = 1111;
    let eaw2Tag = 2222;
    let messageGot = false;

    let handler1 : concurrency.MessageHandler | undefined;
    let handler2 : concurrency.MessageHandler | undefined;

    handler1 = new concurrency.MessageHandler((msg: concurrency.Message) => {
        arktest.assertEQ(msg.getWhat(), eaw2Tag);
        if (msg.getObject() != undefined) {
            let obj = msg.getObject() as string;
            arktest.assertEQ(obj, "message from eaw2");
            handler2?.sendMessage(new concurrency.Message(eaw1Tag, "message from eaw1", handler2!));
        }
    }, eaw);

    handler2 = new concurrency.MessageHandler((msg: concurrency.Message) => {
        arktest.assertEQ(msg.getWhat(), eaw1Tag);
        arktest.assertEQ(msg.getObject(), "message from eaw1");
        messageGot = true;
        waiter.Fire();
    }, eaw2);

    handler1?.sendMessage(new concurrency.Message(eaw2Tag, "message from eaw2", handler1!));

    waiter.Wait();
    arktest.assertEQ(messageGot, true);
    eaw2.quit();
}

function eworkerInteropTest() {
    eaw.start();
    let eworkerInteropSuite = new arktest.ArkTestsuite('eworker.InteropTest');
    eworkerInteropSuite.addAsyncTest('testProcessInput', testProcessInput);
    eworkerInteropSuite.addAsyncTest('testCreateArray', testCreateArray);
    eworkerInteropSuite.addAsyncTest('testGetUser', testGetUser);
    eworkerInteropSuite.addAsyncTest('testUnion', testUnion);
    eworkerInteropSuite.addAsyncTest('testBaseClass', testBaseClass);
    eworkerInteropSuite.addAsyncTest('testChildClass', testChildClass);
    eworkerInteropSuite.addAsyncTest('testBaseFunc', testBaseFunc);
    eworkerInteropSuite.addAsyncTest('testChildFunc', testChildFunc);
    eworkerInteropSuite.addAsyncTest('testFooBaseObjInvoke', testFooBaseObjInvoke);
    eworkerInteropSuite.addAsyncTest('testFooBaseObjInvokeMethod', testFooBaseObjInvokeMethod);
    eworkerInteropSuite.addAsyncTest('testFooChildObjInvoke', testFooChildObjInvoke);
    eworkerInteropSuite.addAsyncTest('testFooChildObjInvokeMethod', testFooChildObjInvokeMethod);
    eworkerInteropSuite.addAsyncTest('testArrowFuncInvoke', testArrowFuncInvoke);
    eworkerInteropSuite.addAsyncTest('testOptionalParams', testOptionalParams);
    eworkerInteropSuite.addAsyncTest('testError', testError);
    eworkerInteropSuite.addAsyncTest('testThrowError', testThrowError);
    eworkerInteropSuite.addAsyncTest('testCustomThrowError', testCustomThrowError);
    eworkerInteropSuite.addAsyncTest('testEAWorkerNestedExecute', testEAWorkerNestedExecute);
    eworkerInteropSuite.addAsyncTest('testEAWorkerInteropLoop', testEAWorkerInteropLoop);
    eworkerInteropSuite.addAsyncTest('testEAWorkerSerialize', testEAWorkerSerialize);
    eworkerInteropSuite.addAsyncTest('testEAWorkerSerializeClassInstantiate', testEAWorkerSerializeClassInstantiate);
    eworkerInteropSuite.addAsyncTest('testEAWorkerSerializeUndefined', testEAWorkerSerializeUndefined);
    eworkerInteropSuite.addAsyncTest('testEAWorkerSerializeNull', testEAWorkerSerializeNull);
    eworkerInteropSuite.addTest('testEAWorkerMessageCancellation', testEAWorkerMessageCancellation);
    eworkerInteropSuite.addTest('testRemoveMessageHandlerTest', removeMessageHandlerTest);
    eworkerInteropSuite.addTest('testEAWorkerCommmunicateWithMessage', testEAWorkerCommmunicateWithMessage);
    const res = eworkerInteropSuite.run();
    eaw.quit();
    return res == 0;
}
