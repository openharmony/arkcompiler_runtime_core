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

import {
    getPromiseNumber,
    getPromiseString,
    getPromiseBoolean,
    getPromiseBigInt,
    getPromiseNull,
    getPromiseUndefined,
    getPromiseNan,
    getPromiseInfinity,
    getRejectPromise,
    getFulfilled1,
    getFulfilled2,
    getFulfilled3,
    getRejected1,
    getRejected2,
    createAllEmpty,
    createAllWithPromises,
    createAllWithRejected,
    createAllMixed,
    createAllWithString,
    createAnyEmpty,
    createAnyAllRejected,
    createAnyMixed,
    createAnyFirstWins,
    createRaceEmpty,
    createRacePlainValues,
    createRaceFulfilledWins,
    createRaceRejectedWins,
    createRaceMixedImmediateWins,
    createRaceRejectImmediate,
    createRaceWithString,
    cbLambda,
    CreateClass1,
} from 'tsPromiseAllModule';

function assertWithLog(condition: boolean, testName: string): void {
    console.log(testName + (condition ? ' success!' : ' failed!'));
    arktest.assertTrue(condition);
}

async function checkPromiseNumber(): Promise<void> {
    let res: number = await getPromiseNumber();
    assertWithLog(res === 42, 'checkPromiseNumber');
}

async function checkPromiseString(): Promise<void> {
    let res = await getPromiseString();
    assertWithLog(res === 'hello', 'checkPromiseString');
}

async function checkPromiseBoolean(): Promise<void> {
    let res: boolean = await getPromiseBoolean();
    assertWithLog(res === true, 'checkPromiseBoolean');
}

async function checkPromiseBigInt(): Promise<void> {
    let res: bigint = await getPromiseBigInt();
    assertWithLog(res === 123456789n, 'checkPromiseBigInt');
}

async function checkPromiseNull(): Promise<void> {
    let res: null = await getPromiseNull();
    assertWithLog(res === null, 'checkPromiseNull');
}

async function checkPromiseUndefined(): Promise<void> {
    let res: undefined = await getPromiseUndefined();
    assertWithLog(res === undefined, 'checkPromiseUndefined');
}

async function checkPromiseNan(): Promise<void> {
    let res: number = await getPromiseNan();
    assertWithLog(isNaN(res), 'checkPromiseNan');
}

async function checkPromiseInfinity(): Promise<void> {
    let res: number = await getPromiseInfinity();
    assertWithLog(res === Infinity, 'checkPromiseInfinity');
}

async function checkThenTwoParamsFulfilled(): Promise<void> {
    let res = await getPromiseNumber()
        .then(
            (v) => 'ok:' + v.toString(),
            (e: Error) => 'err'
        );
    assertWithLog(res === 'ok:42', 'checkThenTwoParamsFulfilled');
}

async function checkThenTwoParamsRejected(): Promise<void> {
    let res = await getRejectPromise()
        .then(
            (v) => 'wrong',
            (e: Error) => e.message
        );
    assertWithLog(res === 'error', 'checkThenTwoParamsRejected');
}

async function checkThenOnlyOnFulfilled(): Promise<void> {
    let res = await getPromiseNumber()
        .then((v) => v * 2);
    assertWithLog(res === 84, 'checkThenOnlyOnFulfilled');
}

async function checkThenPromiseUndefined(): Promise<void> {
    let res = await getPromiseNumber()
        .then(() => Promise.resolve('hello'));
    assertWithLog(res === 'hello', 'checkThenPromiseUndefined');
}

async function checkThenNoParamsRejected(): Promise<void> {
    let caught = false;
    try {
        await getRejectPromise()
            .then(undefined)
            .then((v) => v);
    } catch (e) {
        caught = (e as Error).message === 'error';
    }
    assertWithLog(caught, 'checkThenNoParamsRejected');
}

async function checkCatchStandardRejected(): Promise<void> {
    let res = await getRejectPromise()
        .catch()
        .catch((e: Error) => e.message);
    assertWithLog(res === 'error', 'checkCatchStandardRejected');
}

async function checkCatchFulfilledPassthrough(): Promise<void> {
    let res = await getPromiseNumber()
        .catch()
        .then((v) => v * 2);
    assertWithLog(res === 84, 'checkCatchFulfilledPassthrough');
}

async function checkCatchReturnPromise(): Promise<void> {
    let res = await getRejectPromise()
        .catch((e: Error) => Promise.resolve<string>('cached'))
    assertWithLog(res === 'cached', 'checkCatchReturnPromise');
}

async function checkCatchThrowChain(): Promise<void> {
    let res = await getRejectPromise()
        .catch(() => { throw new Error('errorB'); })
        .catch((e: Error) => e.message);
    assertWithLog(res === 'errorB', 'checkCatchThrowChain');
}

async function checkFinallyFulfilled(): Promise<void> {
    let loading = false;
    let res = await getPromiseNumber()
        .then((v) => v * 2)
        .finally(() => { loading = true; });
    assertWithLog(loading === true && res === 84, 'checkFinallyFulfilled');
}

async function checkFinallyRejectedFromTS(): Promise<void> {
    let finallyCalled = false;
    let res = await getRejectPromise()
        .catch((e: Error) => e.message)
        .finally(() => { finallyCalled = true; });
    assertWithLog(finallyCalled && res === 'error', 'checkFinallyRejectedFromTS');
}

async function checkFinallyNoParam(): Promise<void> {
    let res = await getPromiseNumber()
        .finally()
        .then((v) => v * 2);
    assertWithLog(res === 84, 'checkFinallyNoParam');
}

async function checkFinallyThrowOverridesFulfilled(): Promise<void> {
    let res = await getPromiseString()
        .finally((): void => { throw new Error('hi'); })
        .then((v) => v)
        .catch((e: Error) => e.message);
    assertWithLog(res === 'hi', 'checkFinallyThrowOverridesFulfilled');
}

async function checkFinallyThrowOverridesRejected(): Promise<void> {
    let res = await getRejectPromise()
        .finally((): void => { throw new Error('new_error'); })
        .catch((e: Error) => e.message);
    assertWithLog(res === 'new_error', 'checkFinallyThrowOverridesRejected');
}

async function checkFinallyReturnValueIgnored(): Promise<void> {
    let res = await getPromiseString()
        .finally((): number => { return 999; })
        .then<string>((v) => v);
    assertWithLog(res === 'hello', 'checkFinallyReturnValueIgnored');
}

async function checkResolvePromiseResolve(): Promise<void> {
    let res = await Promise.resolve(getPromiseNumber());
    assertWithLog(res === 42, 'checkResolvePromiseResolve');
}

async function checkResolvePromiseReject(): Promise<void> {
    let res = await Promise.resolve(getRejectPromise())
        .catch((e: Error) => e.message);
    assertWithLog(res === 'error', 'checkResolvePromiseReject');
}

async function checkAllFulfilled(): Promise<void> {
    let res = await Promise.all([getFulfilled1(), getFulfilled2(), getFulfilled3(), 4]);
    let valid = res.length === 4 && res[0] === 1 && res[1] === 2 && res[2] === 3 && res[3] === 4;
    assertWithLog(valid, 'checkAllFulfilled');
}

async function checkAllRejected(): Promise<void> {
    let res = await Promise.all([getFulfilled1(), getRejected1(), getFulfilled2(), getRejected2()])
        .catch((e: Error) => e.message);
    assertWithLog(res === 'error1', 'checkAllRejected');
}

async function checkStaticAllEmpty(): Promise<void> {
    let res = await createAllEmpty();
    let valid = res.length === 0;
    assertWithLog(valid, 'checkStaticAllEmpty');
}

async function checkStaticAllWithPromises(): Promise<void> {
    let res = await createAllWithPromises();
    let valid = res.length === 3 && res[0] === 10 && res[1] === 20 && res[2] === 30;
    assertWithLog(valid, 'checkStaticAllWithPromises');
}

async function checkStaticAllWithRejected(): Promise<void> {
    let res = await createAllWithRejected()
        .catch((e: Error) => e.message);
    assertWithLog(res === 'error', 'checkStaticAllWithRejected');
}

async function checkStaticAllMixed(): Promise<void> {
    let res = await createAllMixed();
    let valid = res.length === 3 && res[0] === 'a' && res[1] === 123 && res[2] === 'c';
    assertWithLog(valid, 'checkStaticAllMixed');
}

async function checkStaticAllWithString(): Promise<void> {
    let res = await createAllWithString();
    let valid = res.length === 2 && res[0] === 'a' && res[1] === 'b';
    assertWithLog(valid, 'checkStaticAllWithString');
}

async function checkStaticAnyEmpty(): Promise<void> {
    let res: boolean = false;
    try {
        await createAnyEmpty();
    } catch (e) {
        res = e instanceof Error;
    }
    assertWithLog(res, 'checkStaticAnyEmpty');
}

async function checkAnyFulfilled(): Promise<void> {
    let res = await Promise.any([getFulfilled1(), getFulfilled2(), getRejectPromise()]);
    assertWithLog(res === 1, 'checkAnyFulfilled');
}

async function checkStaticAnyAllRejected(): Promise<void> {
    let errors = await createAnyAllRejected();
    let valid = errors.length === 3 && errors[0] === 'A' && errors[1] === 'B' && errors[2] === 'C';
    assertWithLog(valid, 'checkStaticAnyAllRejected');
}

async function checkStaticAnyMixed(): Promise<void> {
    let res: number = await createAnyMixed();
    assertWithLog(res === 100, 'checkStaticAnyMixed');
}

async function checkStaticAnyFirstWins(): Promise<void> {
    let res = await createAnyFirstWins();
    assertWithLog(res === 'fast', 'checkStaticAnyFirstWins');
}

async function checkRaceFulfilled(): Promise<void> {
    let res = await Promise.race([getFulfilled2(), getFulfilled1(), getFulfilled3()]);
    assertWithLog(res === 1, 'checkRaceFulfilled');
}

async function checkRaceRejected(): Promise<void> {
    let res = await Promise.race([getFulfilled1(), getFulfilled2(), getRejectPromise()])
        .catch((e:Error) => e.message);
    assertWithLog(res === 'error', 'checkRaceRejected');
}

async function checkStaticRaceEmpty(): Promise<void> {
    let settled = false;
    createRaceEmpty().then(() => { settled = true; })
        .catch(() => { settled = true; });
    assertWithLog(!settled, 'checkStaticRaceEmpty');
}

async function checkStaticRacePlainValues(): Promise<void> {
    let res = await createRacePlainValues();
    assertWithLog(res === 1, 'checkStaticRacePlainValues');
}

async function checkStaticRaceFulfilledWins(): Promise<void> {
    let res = await createRaceFulfilledWins();
    assertWithLog(res === 'fast', 'checkStaticRaceFulfilledWins');
}

async function checkStaticRaceRejectedWins(): Promise<void> {
    let caught = false;
    try {
        await createRaceRejectedWins();
    } catch (e) {
        caught = (e as Error).message === 'fail';
    }
    assertWithLog(caught, 'checkStaticRaceRejectedWins');
}

async function checkStaticRaceMixedImmediateWins(): Promise<void> {
    let res = await createRaceMixedImmediateWins();
    assertWithLog(res === 42, 'checkStaticRaceMixedImmediateWins');
}

async function checkStaticRaceRejectImmediate(): Promise<void> {
    let caught = false;
    try {
        await createRaceRejectImmediate();
    } catch (e) {
        caught = (e as Error).message === 'immediate_fail';
    }
    assertWithLog(caught, 'checkStaticRaceRejectImmediate');
}

async function checkStaticRaceWithString(): Promise<void> {
    let res = await createRaceWithString();
    assertWithLog(res === 'a', 'checkStaticRaceWithString');
}

function checkcbLambdaNoWait(): void {
    let res = cbLambda();
    assertWithLog(res instanceof Promise, 'checkcbLambdaNoWait');
}

async function checkcbLambda(): Promise<void> {
    let res = await cbLambda();
    assertWithLog(res === 24, 'checkcbLambda');
}

let c = new CreateClass1;

function checkInstanceMethodNoWait(): void {
    let p = c.foo();
    assertWithLog(p instanceof Promise, 'checkInstanceMethodNoWait');
}

async function checkInstanceMethod(): Promise<void> {
    let res = await c.foo();
    assertWithLog(res === 'Hello from foo!', 'checkInstanceMethod');
}

function checkStaticMethodNoWait(): void {
    let p = CreateClass1.foo1();
    assertWithLog(p instanceof Promise, 'checkStaticMethodNoWait');
}

async function checkStaticMethod(): Promise<void> {
    let res = await CreateClass1.foo1();
    assertWithLog(res === 'Hello!', 'checkStaticMethod');
}
