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

let eaw = new EAWorker(true);

const MODULE_NAME = '../../plugins/ets/tests/interop_js/tests/test_esvalue_thread_safety/esvalue_thread_safety_helpers';
const interopCtxErrorMessage: string = 'Interop object must be used in the same InteropCtx as it was created';
let mainThreadModule: ESValue;

function testBasicOperationsInWorker(): void {
    let callback = () => {
        let module = ESValue.load(MODULE_NAME);
        let numProp = module.getProperty('numVal');
        arktest.assertEQ(numProp.toNumber(), 42);

        let strProp = module.getProperty('strVal');
        arktest.assertEQ(strProp.toString(), 'hello_world');

        let boolProp = module.getProperty('boolVal');
        arktest.assertTrue(boolProp.toBoolean());

        let undefProp = module.getProperty('undefVal');
        arktest.assertTrue(undefProp.isUndefined());

        let nullProp = module.getProperty('nullVal');
        arktest.assertTrue(nullProp.isNull());
    };
    eaw.run<void>(callback).Await();
}

function testWrapMethodsInWorker(): void {
    let callback = () => {
        let numEv = ESValue.wrapNumber(42);
        arktest.assertTrue(numEv.isNumber());
        arktest.assertEQ(numEv.toNumber(), 42);

        let strEv = ESValue.wrapString('hello');
        arktest.assertTrue(strEv.isString());
        arktest.assertEQ(strEv.toString(), 'hello');

        let boolEv = ESValue.wrapBoolean(true);
        arktest.assertTrue(boolEv.isBoolean());
        arktest.assertTrue(boolEv.toBoolean());

        let bigIntEv = ESValue.wrapBigInt(123n);
        arktest.assertTrue(bigIntEv.isBigInt());
        arktest.assertEQ(bigIntEv.toBigInt(), 123n);

        let undefEv = ESValue.Undefined;
        arktest.assertTrue(undefEv.isUndefined());

        let nullEv = ESValue.Null;
        arktest.assertTrue(nullEv.isNull());

        let floatEv = ESValue.wrapFloat(3.14 as float);
        arktest.assertTrue(floatEv.isNumber());

        let doubleEv = ESValue.wrapDouble(2.718);
        arktest.assertTrue(doubleEv.isNumber());
    };
    eaw.run<void>(callback).Await();
}

function testTypeChecksInWorker(): void {
    let callback = () => {
        let module = ESValue.load(MODULE_NAME);

        let numEv = module.getProperty('numVal');
        arktest.assertTrue(numEv.isNumber());
        arktest.assertEQ(numEv.typeOf(), 'number');

        let strEv = module.getProperty('strVal');
        arktest.assertTrue(strEv.isString());
        arktest.assertEQ(strEv.typeOf(), 'string');

        let boolEv = module.getProperty('boolVal');
        arktest.assertTrue(boolEv.isBoolean());
        arktest.assertEQ(boolEv.typeOf(), 'boolean');

        let objEv = module.getProperty('testObject');
        arktest.assertTrue(objEv.isObject());
        arktest.assertEQ(objEv.typeOf(), 'object');

        let funcEv = module.getProperty('add');
        arktest.assertTrue(funcEv.isFunction());
        arktest.assertEQ(funcEv.typeOf(), 'function');

        let undefEv = module.getProperty('undefVal');
        arktest.assertTrue(undefEv.isUndefined());
        arktest.assertEQ(undefEv.typeOf(), 'undefined');

        let nullEv = module.getProperty('nullVal');
        arktest.assertTrue(nullEv.isNull());
        arktest.assertEQ(nullEv.typeOf(), 'object');
    };
    eaw.run<void>(callback).Await();
}

function testPropertyAccessInWorker(): void {
    let callback = () => {
        let module = ESValue.load(MODULE_NAME);
        let obj = module.getProperty('testObject');

        arktest.assertEQ(obj.getProperty('propNum').toNumber(), 100);
        arktest.assertEQ(obj.getProperty('propStr').toString(), 'test_string');
        arktest.assertFalse(obj.getProperty('propBool').toBoolean());

        let arr = module.getProperty('testArray');
        arktest.assertEQ(arr.getProperty(0).toNumber(), 10);
        arktest.assertEQ(arr.getProperty(2).toNumber(), 30);

        let safeVal = obj.getPropertySafe('nonExistent');
        arktest.assertTrue(safeVal.isUndefined());

        obj.setProperty('propNum', ESValue.wrapNumber(999));
        arktest.assertEQ(obj.getProperty('propNum').toNumber(), 999);

        obj.setProperty('newProp', ESValue.wrapString('new_value'));
        arktest.assertEQ(obj.getProperty('newProp').toString(), 'new_value');

        arktest.assertTrue(obj.hasProperty('propNum'));
        arktest.assertFalse(obj.hasProperty('nonExistent'));
    };
    eaw.run<void>(callback).Await();
}

function testInvocationInWorker(): void {
    let callback = () => {
        let module = ESValue.load(MODULE_NAME);

        let addFn = module.getProperty('add');
        let res = addFn.invoke(ESValue.wrapNumber(3), ESValue.wrapNumber(4));
        arktest.assertEQ(res.toNumber(), 7);

        let testClass = module.getProperty('ThreadTestClass');
        let instance = testClass.instantiate(ESValue.wrapNumber(1), ESValue.wrapString('Alice'));
        let greetRes = instance.invokeMethod('greet');
        arktest.assertEQ(greetRes.toString(), 'Hello, Alice!');

        let idRes = instance.invokeMethod('getId');
        arktest.assertEQ(idRes.toNumber(), 1);

        let func = module.getProperty('testObject').getProperty('propFunc');
        let recvResult = func.invokeWithRecv(module.getProperty('testObject'), ESValue.wrapNumber(5));
        arktest.assertEQ(recvResult.toNumber(), 15);
    };
    eaw.run<void>(callback).Await();
}

function testIterationInWorker(): void {
    let callback = () => {
        let module = ESValue.load(MODULE_NAME);
        let obj = module.getProperty('iterableObj');

        let keyIter = obj.keys();
        let keyNext = keyIter.next();
        arktest.assertFalse(keyNext.done);

        let valIter = obj.values();
        let valNext = valIter.next();
        arktest.assertFalse(valNext.done);

        let entryIter = obj.entries();
        let entryNext = entryIter.next();
        arktest.assertFalse(entryNext.done);
    };
    eaw.run<void>(callback).Await();
}

function testEqualityInWorker(): void {
    let callback = () => {
        let ev1 = ESValue.wrapNumber(42);
        let ev2 = ESValue.wrapNumber(42);

        arktest.assertTrue(ESValue.areEqual(ev1, ev2));
        arktest.assertTrue(ESValue.areStrictlyEqual(ev1, ev2));
        arktest.assertTrue(ESValue.areEqualSafe(ev1, ev2));
        arktest.assertTrue(ESValue.areEqualSafe(ESValue.Undefined, ESValue.Null));

        arktest.assertTrue(ev1.isEqualTo(ev2));
        arktest.assertTrue(ev1.isStrictlyEqualTo(ev2));
        arktest.assertTrue(ev1.isEqualToSafe(ev2));
    };
    eaw.run<void>(callback).Await();
}

function testCrossInteropCtxIsolation(): void {
    mainThreadModule = ESValue.load(MODULE_NAME);
    arktest.assertEQ(mainThreadModule.getProperty('numVal').toNumber(), 42);

    let callback = () => {
        mainThreadModule.getProperty('numVal');
    };

    let threw = false;
    try {
        eaw.run<void>(callback).Await();
    } catch (error) {
        threw = (error instanceof Error) && (error.message.indexOf(interopCtxErrorMessage) !== -1);
    }
    arktest.assertTrue(threw);
}

function testESValueSerialization(): void {
    let module = ESValue.load(MODULE_NAME);
    let serializeObj = module.getProperty('testArray');

    let callback = (value: ESValue) => {
        arktest.assertEQ(value.getProperty(0).toNumber(), 10);
        arktest.assertEQ(value.getProperty(2).toNumber(), 30);
        return true;
    };

    let res = eaw.run<boolean>(callback, serializeObj).Await() as boolean;
    arktest.assertTrue(res);
}

function testNestedWorkers(): void {
    let callback = (): boolean => {
        let innerWorker = new EAWorker(true);
        innerWorker.start();

        let module = ESValue.load(MODULE_NAME);
        let res1 = module.getProperty('add').invoke(ESValue.wrapNumber(3), ESValue.wrapNumber(4));
        arktest.assertEQ(res1.toNumber(), 7);

        let innerCallback = (): boolean => {
            let innerModule = ESValue.load(MODULE_NAME);
            let res = innerModule.getProperty('add').invoke(ESValue.wrapNumber(5), ESValue.wrapNumber(6));
            return res.toNumber() === 11;
        };

        let resInner = innerWorker.run<boolean>(innerCallback).Await() as boolean;
        innerWorker.join();
        arktest.assertTrue(resInner);
        return resInner;
    };

    let res = eaw.run<boolean>(callback).Await() as boolean;
    arktest.assertTrue(res);
}

function testConcurrentWorkers(): void {
    let eaw2 = new EAWorker(true);
    eaw2.start();

    let callback1 = (): boolean => {
        let module = ESValue.load(MODULE_NAME);
        let res = module.getProperty('add').invoke(ESValue.wrapNumber(1), ESValue.wrapNumber(2));
        return res.toNumber() === 3;
    };

    let callback2 = (): boolean => {
        let module = ESValue.load(MODULE_NAME);
        let res = module.getProperty('multiply').invoke(ESValue.wrapNumber(3), ESValue.wrapNumber(4));
        return res.toNumber() === 12;
    };

    let res1 = eaw.run<boolean>(callback1).Await() as boolean;
    let res2 = eaw2.run<boolean>(callback2).Await() as boolean;
    eaw2.join();

    arktest.assertTrue(res1);
    arktest.assertTrue(res2);
}

function testMultipleRunsOnSameWorker(): void {
    let callback1 = (): boolean => {
        let module = ESValue.load(MODULE_NAME);
        let res = module.getProperty('add').invoke(ESValue.wrapNumber(1), ESValue.wrapNumber(2));
        return res.toNumber() === 3;
    };

    let callback2 = (): boolean => {
        let module = ESValue.load(MODULE_NAME);
        let res = module.getProperty('multiply').invoke(ESValue.wrapNumber(3), ESValue.wrapNumber(4));
        return res.toNumber() === 12;
    };

    let res1 = eaw.run<boolean>(callback1).Await() as boolean;
    arktest.assertTrue(res1);

    let res2 = eaw.run<boolean>(callback2).Await() as boolean;
    arktest.assertTrue(res2);
}

function testStressLoop(): void {
    const ITERATIONS = 1000;
    let callback = (): boolean => {
        let module = ESValue.load(MODULE_NAME);
        let addFn = module.getProperty('add');
        let obj = module.getProperty('testObject');

        for (let i = 0; i < ITERATIONS; i++) {
            let res = addFn.invoke(ESValue.wrapNumber(i), ESValue.wrapNumber(1));
            arktest.assertEQ(res.toNumber(), i + 1);

            obj.setProperty('propNum', ESValue.wrapNumber(i));
            arktest.assertEQ(obj.getProperty('propNum').toNumber(), i);
        }
        return true;
    };

    let res = eaw.run<boolean>(callback).Await() as boolean;
    arktest.assertTrue(res);
}

function testTaskpoolBasic(): void {
    let callback = (): boolean => {
        let module = ESValue.load(MODULE_NAME);
        let res = module.getProperty('add').invoke(ESValue.wrapNumber(3), ESValue.wrapNumber(4));
        return res.toNumber() === 7;
    };

    let res = waitForCompletion(() => taskpool.execute(callback)) as boolean;
    arktest.assertTrue(res);
}

function testTaskpoolConcurrent(): void {
    let callback1 = (): boolean => {
        let module = ESValue.load(MODULE_NAME);
        let res = module.getProperty('add').invoke(ESValue.wrapNumber(1), ESValue.wrapNumber(2));
        return res.toNumber() === 3;
    };

    let callback2 = (): boolean => {
        let module = ESValue.load(MODULE_NAME);
        let res = module.getProperty('multiply').invoke(ESValue.wrapNumber(3), ESValue.wrapNumber(4));
        return res.toNumber() === 12;
    };

    let res1 = waitForCompletion(() => taskpool.execute(callback1)) as boolean;
    let res2 = waitForCompletion(() => taskpool.execute(callback2)) as boolean;

    arktest.assertTrue(res1);
    arktest.assertTrue(res2);
}

function testTaskpoolStress(): void {
    const ITERATIONS = 500;
    let callback = (): boolean => {
        let module = ESValue.load(MODULE_NAME);
        let addFn = module.getProperty('add');
        let obj = module.getProperty('testObject');

        for (let i = 0; i < ITERATIONS; i++) {
            let res = addFn.invoke(ESValue.wrapNumber(i), ESValue.wrapNumber(1));
            arktest.assertEQ(res.toNumber(), i + 1);

            obj.setProperty('propNum', ESValue.wrapNumber(i));
            arktest.assertEQ(obj.getProperty('propNum').toNumber(), i);
        }
        return true;
    };

    let res = waitForCompletion(() => taskpool.execute(callback)) as boolean;
    arktest.assertTrue(res);
}

function esvalueThreadSafetyTest(): boolean {
    eaw.start();
    let suite = new arktest.ArkTestsuite('ESValue.ThreadSafety');
    suite.addTest('testBasicOperationsInWorker', testBasicOperationsInWorker);
    suite.addTest('testWrapMethodsInWorker', testWrapMethodsInWorker);
    suite.addTest('testTypeChecksInWorker', testTypeChecksInWorker);
    suite.addTest('testPropertyAccessInWorker', testPropertyAccessInWorker);
    suite.addTest('testInvocationInWorker', testInvocationInWorker);
    suite.addTest('testIterationInWorker', testIterationInWorker);
    suite.addTest('testEqualityInWorker', testEqualityInWorker);
    suite.addTest('testCrossInteropCtxIsolation', testCrossInteropCtxIsolation);
    suite.addTest('testESValueSerialization', testESValueSerialization);
    suite.addTest('testNestedWorkers', testNestedWorkers);
    suite.addTest('testConcurrentWorkers', testConcurrentWorkers);
    suite.addTest('testMultipleRunsOnSameWorker', testMultipleRunsOnSameWorker);
    suite.addTest('testStressLoop', testStressLoop);
    suite.addTest('testTaskpoolBasic', testTaskpoolBasic);
    suite.addTest('testTaskpoolConcurrent', testTaskpoolConcurrent);
    suite.addTest('testTaskpoolStress', testTaskpoolStress);
    const res = suite.run();
    eaw.join();
    CoroutineExtras.stopTaskpool();
    return res == 0;
}
