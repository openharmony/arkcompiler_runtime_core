/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
let etsVm = globalThis.gtest.etsVm;
let checkToPromise = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkToPromise');
let checkToNumber = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkToNumber');
let checkToString = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkToString');
let checkToBoolean = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkToBoolean');
let checkToBigInt = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkToBigInt');
let testInteropRejectStringCatch = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'testInteropRejectStringCatch');
let testInteropRejectStringThen = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'testInteropRejectStringThen');
let testInteropFinally = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'testInteropFinally');
let testInteropAll = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'testInteropAll');
let checkNestedPromise = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkNestedPromise');
let checkRaceTest = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkRaceTest');
let checkAllSettledTest = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkAllSettledTest');
let checkChainTest = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkChainTest');
let checkObjectPromise = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkObjectPromise');
let checkArrayPromise = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkArrayPromise');
let checkNanPromise = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkNanPromise');
let checkInfinityPromise = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkInfinityPromise');
let checkNullPromise = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkNullPromise');
let checkUndefinedPromise = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkUndefinedPromise');
let checkThrowTypeError = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkThrowTypeError');
let checkThrowNullError = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkThrowNullError');
let checkAwaitCatch = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkAwaitCatch');
let checkTimeoutSuccess = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkTimeoutSuccess');
let checkTimeoutFail = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkTimeoutFail');

export function sleep(ms: number): Promise<void> {
    return new Promise<void>((resolve) => {
        setTimeout(resolve, ms);
    });
}

export async function sleepRetNumber(ms: number): Promise<number> {
    await sleep(ms);
    return 0xcafe;
}

export async function getPromiseNumber(): Promise<number> {
    return Promise.resolve(42);
}

export async function getPromiseString(): Promise<string> {
    return Promise.resolve(' abc ');
}

export async function getPromiseBoolean(): Promise<boolean> {
    return Promise.resolve(true);
}

export async function getPromiseBigInt(): Promise<bigint> {
    return Promise.resolve(123456789n);
}

export async function asyncFunctionRejectAndFinallyString(): Promise<string> {
    return new Promise<string>((resolve, reject) => {
        setTimeout(() => reject('MyError'), 20)
    });
}

export let asyncArrayFunctionBoolean = async (): Promise<boolean> => {
    return true;
}

export let asyncArrayFunctionBoolean2 = async (): Promise<boolean> => {
    return false;
}

export async function nestedPromise(): Promise<number> {
    return Promise.resolve(Promise.resolve(42));
}

export async function raceTest(): Promise<number> {
    let p1 = new Promise<number>((resolve) => setTimeout(() => resolve(1), 100));
    let p2 = new Promise<number>((resolve) => setTimeout(() => resolve(2), 50));
    return Promise.race([p1, p2]);
}

export async function allSettledTest(): Promise<any> {
    let p1 = Promise.resolve(1);
    let p2 = Promise.reject<string>('error');
    return Promise.allSettled([p1, p2]);
}

export async function chainTest(): Promise<number> {
    return Promise.resolve(1)
        .then(x => x + 1)
        .then(x => x * 2)
        .then(x => x);
}

export async function objectPromise(): Promise<Object> {
    return Promise.resolve({ name: 'test', count: 42 });
}

export async function arrayPromise(): Promise<Array<number>> {
    return Promise.resolve([1, 2, 3]);
}

export async function nanPromise(): Promise<number> {
    return Promise.resolve(NaN);
}

export async function infinityPromise(): Promise<number> {
    return Promise.resolve(Infinity);
}

export async function nullPromise(): Promise<null> {
    return Promise.resolve(null);
}

export async function undefinedPromise(): Promise<undefined> {
    return Promise.resolve(undefined);
}

export async function throwError(): Promise<number> {
    throw new TypeError('Test error');
}

export async function throwNullError(): Promise<number> {
    throw null;
}

export async function awaitCatch(): Promise<number> {
    try {
        await Promise.reject<string>('error');
        return 0;
    } catch (e) {
        return 1;
    }
}

export async function timeoutSuccessTest(): Promise<number> {
    const timeout = new Promise<number>((_, reject) =>
        setTimeout(() => reject('timeout'), 200)
    );
    const task = Promise.resolve(42);
    return Promise.race([task, timeout]);
}

export async function timeoutFailTest(): Promise<number> {
    const timeout = new Promise<number>((_, reject) =>
        setTimeout(() => reject('timeout'), 50)
    );
    const task = new Promise<number>((resolve) =>
        setTimeout(() => resolve(100), 100)
    );
    return Promise.race([task, timeout]);
}

async function main(): Promise<void> {
    await checkToPromise();
    await checkToNumber();
    await checkToString();
    await checkToBoolean();
    await checkToBigInt();
    testInteropRejectStringCatch();
    testInteropRejectStringThen();
    testInteropFinally();
    testInteropAll();
    checkNestedPromise();
    checkRaceTest();
    checkAllSettledTest();
    checkChainTest();
    checkObjectPromise();
    checkArrayPromise();
    checkNanPromise();
    checkInfinityPromise();
    checkNullPromise();
    checkUndefinedPromise();
    checkThrowTypeError();
    checkThrowNullError();
    checkAwaitCatch();
    checkTimeoutSuccess();
    checkTimeoutFail();
}

main();