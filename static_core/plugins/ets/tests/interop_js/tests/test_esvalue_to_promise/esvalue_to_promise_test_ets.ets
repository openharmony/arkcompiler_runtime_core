/*
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

function assertWithLog(condition: boolean, testName: string): void {
    console.log(testName + (condition ? ' success!' : ' failed!'));
    arktest.assertTrue(condition);
}

async function checkToPromise(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let sleepRetNumber = module.getProperty('sleepRetNumber');
    let p = sleepRetNumber.invoke(ESValue.wrapNumber(5000)).toPromise();
    let res = await p;
    assertWithLog(res.toNumber() === 0xcafe, 'checkToPromise');
}

async function checkToNumber(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let getPromiseNumber = module.getProperty('getPromiseNumber');
    let p = getPromiseNumber.invoke().toPromise();
    let res = false;
    await p.then((value: ESValue) => {
        res = value.toNumber() === 42;
    }).catch((e: Error) => {
        res = false;
    });
    assertWithLog(res, 'checkToNumber');
}

async function checkToString(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let getPromiseString = module.getProperty('getPromiseString');
    let p = getPromiseString.invoke().toPromise();
    let res = false;
    await p.then((value: ESValue) => {
        res = value.toString() === ' abc ';
    }).catch((e: Error) => {
        res = false;
    });
    assertWithLog(res, 'checkToString');
}

async function checkToBoolean(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let getPromiseBoolean = module.getProperty('getPromiseBoolean');
    let p = getPromiseBoolean.invoke().toPromise();
    let res = false;
    await p.then((value: ESValue) => {
        res = value.toBoolean();
    }).catch((e: Error) => {
        res = false;
    });
    assertWithLog(res, 'checkToBoolean');
}

async function checkToBigInt(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let getPromiseBigInt = module.getProperty('getPromiseBigInt');
    let p = getPromiseBigInt.invoke().toPromise();
    let res = false;
    await p.then((value: ESValue) => {
        res = value.toBigInt() === 123456789n;
    }).catch((e: Error) => {
        res = false;
    });
    assertWithLog(res, 'checkToBigInt');
}

function testInteropRejectStringCatch(): int {
    let module = ESValue.load('esvalue_to_promise_test');
    let func = module.getProperty('asyncFunctionRejectAndFinallyString')
    let rejectedPromise = func.invoke().toPromise()
    rejectedPromise.catch((res: Error): void => {
        if ((res as ESError).getValue().toString() === 'MyError') {
            console.log('reject catch success!');
        } else {
            console.log('reject catch failed!');
            console.log(res.message)
        }
    })
    return 0
}

function testInteropRejectStringThen(): int {
    let module = ESValue.load('esvalue_to_promise_test');
    let func = module.getProperty('asyncFunctionRejectAndFinallyString')
    let rejectedPromise = func.invoke().toPromise()
    rejectedPromise.then(
        (value): void => { },
        (res: Error): void => {
            if ((res as ESError).getValue().toString() === 'MyError') {
                console.log('reject then success!');
            } else {
                console.log('reject then failed!');
                console.log(res.message)
            }
        });
    return 0
}

function testInteropFinally(): int {
    let module = ESValue.load('esvalue_to_promise_test');
    let func = module.getProperty('asyncFunctionRejectAndFinallyString')
    let rejectedPromise = func.invoke().toPromise()
    rejectedPromise.finally((): void => {
        console.log('reject finally success!');
    });
    return 0
}

async function testInteropAll(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let asyncArrayFunctionBoolean = module.getProperty('asyncArrayFunctionBoolean');
    let asyncArrayFunctionBoolean2 = module.getProperty('asyncArrayFunctionBoolean2');
    const results = await Promise.all([asyncArrayFunctionBoolean.invoke().toPromise(), asyncArrayFunctionBoolean2.invoke().toPromise()]);
    assertWithLog(results[0].toBoolean() && !results[1].toBoolean(), 'testInteropAll');
    arktest.assertTrue(results[0].toBoolean());
    arktest.assertTrue(!results[1].toBoolean());
}

async function checkNestedPromise(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let nestedPromiseFunc = module.getProperty('nestedPromise');
    let p = nestedPromiseFunc.invoke().toPromise();
    let res = await p;
    assertWithLog(res.toNumber() === 42, 'checkNestedPromise');
}

async function checkRaceTest(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let raceFunc = module.getProperty('raceTest');
    let p = raceFunc.invoke().toPromise();
    let res = await p;
    assertWithLog(res.toNumber() === 2, 'checkRaceTest');
}

async function checkAllSettledTest(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let allSettledFunc = module.getProperty('allSettledTest');
    let p = allSettledFunc.invoke().toPromise();
    let res = await p;
    let valid = res.getProperty('length').toNumber() === 2;
    let firstResult = res.getProperty('0');
    valid = valid && firstResult.getProperty('status').toString() === 'fulfilled';
    valid = valid && firstResult.getProperty('value').toNumber() === 1;
    let secondResult = res.getProperty('1');
    valid = valid && secondResult.getProperty('status').toString() === 'rejected';
    valid = valid && secondResult.getProperty('reason').toString() === 'error';
    assertWithLog(valid, 'checkAllSettledTest');
    arktest.assertTrue(res.getProperty('length').toNumber() === 2);
    arktest.assertTrue(firstResult.getProperty('status').toString() === 'fulfilled');
    arktest.assertTrue(firstResult.getProperty('value').toNumber() === 1);
    arktest.assertTrue(secondResult.getProperty('status').toString() === 'rejected');
    arktest.assertTrue(secondResult.getProperty('reason').toString() === 'error');
}

async function checkChainTest(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let chainFunc = module.getProperty('chainTest');
    let p = chainFunc.invoke().toPromise();
    let res = await p;
    assertWithLog(res.toNumber() === 4, 'checkChainTest');
}

async function checkObjectPromise(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let objectPromiseFunc = module.getProperty('objectPromise');
    let p = objectPromiseFunc.invoke().toPromise();
    let res = await p;
    let valid = res.isObject();
    let name = res.getProperty('name');
    let count = res.getProperty('count');
    valid = valid && name.toString() === 'test';
    valid = valid && count.toNumber() === 42;
    assertWithLog(valid, 'checkObjectPromise');
    arktest.assertTrue(res.isObject());
    arktest.assertTrue(name.toString() === 'test');
    arktest.assertTrue(count.toNumber() === 42);
}

async function checkArrayPromise(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let arrayPromiseFunc = module.getProperty('arrayPromise');
    let p = arrayPromiseFunc.invoke().toPromise();
    let res = await p;
    assertWithLog(res.getProperty('length').toNumber() === 3, 'checkArrayPromise');
}

async function checkNanPromise(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let nanPromiseFunc = module.getProperty('nanPromise');
    let p = nanPromiseFunc.invoke().toPromise();
    let res = await p;
    assertWithLog(res.isNumber() && isNaN(res.toNumber()), 'checkNanPromise');
    arktest.assertTrue(res.isNumber());
    arktest.assertTrue(isNaN(res.toNumber()));
}

async function checkInfinityPromise(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let infinityPromiseFunc = module.getProperty('infinityPromise');
    let p = infinityPromiseFunc.invoke().toPromise();
    let res = await p;
    assertWithLog(res.isNumber() && res.toNumber() === Infinity, 'checkInfinityPromise');
    arktest.assertTrue(res.isNumber());
    arktest.assertTrue(res.toNumber() === Infinity);
}

async function checkNullPromise(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let nullPromiseFunc = module.getProperty('nullPromise');
    let p = nullPromiseFunc.invoke().toPromise();
    let res = await p;
    assertWithLog(res.isNull(), 'checkNullPromise');
}

async function checkUndefinedPromise(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let undefinedPromiseFunc = module.getProperty('undefinedPromise');
    let p = undefinedPromiseFunc.invoke().toPromise();
    let res = await p;
    assertWithLog(res.isUndefined(), 'checkUndefinedPromise');
}

async function checkThrowTypeError(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let throwFunc = module.getProperty('throwError');
    let p = throwFunc.invoke().toPromise();
    let res = false;
    await p.catch((e: Error) => {
        res = e.message.includes('Test error');
    });
    assertWithLog(res, 'checkThrowTypeError');
}

async function checkThrowNullError(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let throwFunc = module.getProperty('throwNullError');
    let p = throwFunc.invoke().toPromise();
    let res = false;
    await p.catch((e: Error) => {
        e = e as ESError;
        res = e.getValue().isNull();
    });
    assertWithLog(res, 'checkThrowNullError');
}

async function checkAwaitCatch(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let awaitCatchFunc = module.getProperty('awaitCatch');
    let p = awaitCatchFunc.invoke().toPromise();
    let res = await p;
    assertWithLog(res.toNumber() === 1, 'checkAwaitCatch');
}

async function checkTimeoutSuccess(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let timeoutFunc = module.getProperty('timeoutSuccessTest');
    let p = timeoutFunc.invoke().toPromise();
    let res = await p;
    assertWithLog(res.toNumber() === 42, 'checkTimeoutSuccess');
}

async function checkTimeoutFail(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let timeoutFunc = module.getProperty('timeoutFailTest');
    let p = timeoutFunc.invoke().toPromise();
    let res = false;
    await p.catch((e: Error) => {
        e = e as ESError;
        res = e.getValue().toString() === 'timeout';
    });
    assertWithLog(res, 'checkTimeoutFail');
}
