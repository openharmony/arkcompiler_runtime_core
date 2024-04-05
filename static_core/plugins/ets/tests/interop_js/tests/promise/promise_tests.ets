/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

class Test {
    private static UNSET: int = 0;
    private static SUCCESS: int = 1;

    constructor() {
        this.result = Test.UNSET;
    }

    success(): void {
        if (this.result == Test.UNSET) {
            this.result = Test.SUCCESS;
        }
    }

    fail(): void {
        console.println("Test failed");
        throw new Error();
    }

    check(): void {
        if (this.result == Test.SUCCESS) {
            return;
        }
        if (this.result == Test.UNSET) {
            console.println("Test result is not set");
        }
        throw new Error();
    }

    static FATAL: int = 0;
    static ERROR: int = 1;
    static INFO: int = 2;
    // private log_level: int = 5;
    private log_level: int = 1;

    setLogLevel(lvl: int): void {
        this.log_level = lvl;
    }

    msg(s: String, lvl: int): void {
        if (lvl <= this.log_level) {
            console.println(s);
        }
    }

    private result: int;
}
let globalTest: Test | null = null;

function check(): int {
    globalTest!.check();
    return 0;
}

function testPendingPromise(): int {
    globalTest = new Test();
    let p = new Promise<Object>((resolve: (value: Object) => void): void => {
        globalTest!.success()
    });
    globalTest!.check();
    p.then<Object | null>((): Object | null => {
        globalTest!.fail();
        return null;
    });
    p.catch<Object | null>((error: Object | null): Object | null => {
        globalTest!.fail();
        return null;
    });
    // Check callback are not called inside 'then' and 'catch'
    globalTest!.check();
    return 0;
}

function testResolvedPromise(): int {
    let obj = new Object();
    let p = new Promise<Object>((resolve: (value: Object) => void): void => {
        resolve(obj);
    });
    globalTest = new Test();
    p.then<Object | null>((value: Object): Object | null => {
        if (value == obj) {
            globalTest!.success();
        } else {
            globalTest!.fail();
        }
        return null;
    });
    p.catch<Object | null>((err: Object | null): Object | null => {
        globalTest!.fail();
        return null;
    });
    return 0;
}

function testRejectedPromise(): int {
    globalTest = new Test();
    let error = new Error();
    try {
        let p = new Promise<Object>((resolve: (value: Object) => void): void => {
            throw error;
        });
        p.then<Object | null>((value: Object): Object | null => {
            globalTest!.fail();
            return null;
        });
        p.catch<Object | null>((err: Object | null): Object => {
            if (err == error) {
                globalTest!.success();
            } else {
                globalTest!.fail();
            }
        });
    } catch (e: Exception) {
        globalTest!.fail();
    }
    return 0;
}

class ThenBeforeResolveFixture {
    public fn: (value: Object) => void;
    public state = 0;
}

function testThenBeforeResolve(): int {
    globalTest = new Test();
    let fixture = new ThenBeforeResolveFixture();
    let p = new Promise<Object>((resolve: (value: Object) => void): void => {
        fixture.fn = resolve;
    });
    let obj = new Object();
    p.then<Object | null>((value: Object): Object | null => {
        if (value == obj) {
            if (fixture.state == 0) {
                fixture.state = 1;
            } else {
                console.println("Wrong 'then' call order");
                globalTest!.fail();
            }
        } else {
            console.println("Then is called with wrong object");
            globalTest!.fail();
        }
        return null;
    });
    p.then<Object | null>((value: Object): Object | null => {
        if (value == obj) {
            if (fixture.state == 1) {
                globalTest!.success();
            } else {
                console.println("Wrong 'then' call order");
                globalTest!.fail();
            }
        } else {
            console.println("Then is called with wrong object");
            globalTest!.fail();
        }
        return null;
    });
    fixture.fn(obj);
    return 0;
}

function testPromiseEmptyThen(): int {
    globalTest = new Test();
    let p = new Promise<Object>((resolve: (value: Object) => void): void => {
        resolve(new Object());
    });
    let nextP = p.then<Object>((): Object => {
        globalTest!.success();
    });
    return 0;
}

class PromiseChainFixture {
    public state = 0;
}

function testPromiseChain(): int {
    globalTest = new Test();
    let fixture = new PromiseChainFixture();
    let p = new Promise<Object>((resolve: (value: Object) => void): void => {
        resolve(new Object());
    });
    // TODO(audovichenko): Remove p1 and p2 variables
    let p1: Promise<Object | null> = p.then<Object | null>((): Object | null => {
        if (fixture.state == 0) {
            fixture.state = 1;
        } else {
            globalTest!.fail();
        }
        return null;
    });
    let p2: Promise<Object | null> = p1.then<Object | null>((): Object | null => {
        if (fixture.state == 1) {
            fixture.state = 2;
        } else {
            globalTest!.fail();
        }
        return null;
    });
    p2.then<Object | null>((): Object | null => {
        if (fixture.state == 2) {
            globalTest!.success();
        } else {
            globalTest!.fail();
        }
        return null;
    });
    return 0;
}

function testPromiseCatchFinallyChain(): int {
    globalTest = new Test();
    let fixture = new PromiseChainFixture();
    let p = Promise.reject<Object>(new Object());
    // TODO(audovichenko): Remove p1 and p2 variables
    let p1: Promise<Object | null> = p.then<Object | null>((): Object | null => {
        globalTest!.fail();
        return null;
    });
    let p2: Promise<Object | null> = p1.catch<Object | null>((value): Object | null => {
        if (fixture.state == 0) {
            fixture.state = 1;
        } else {
            globalTest!.fail();
        }
        return null;
    });
    p2.finally(() => {
        if (fixture.state == 1) {
            globalTest!.success();
        } else {
            globalTest!.fail();
        }
    });
    return 0;
}

function testResolvePromiseThenFinally(): int {
    globalTest = new Test();
    let fixture = new PromiseChainFixture();
    let p = Promise.resolve<Object>(new Object());
    // TODO(audovichenko): Remove p1 and p2 variables
    let p1: Promise<Object | null> = p.then<Object | null>((value: Object): Object | null => {
        if (fixture.state == 0) {
            fixture.state = 1;
        } else {
            globalTest!.fail();
        }
        return null;
    }, (error: Object | null): Object | null => {
        globalTest!.fail();
        return null;
    });
    p1.finally(() => {
        if (fixture.state == 1) {
            globalTest!.success();
        } else {
            globalTest!.fail();
        }
    });
    return 0;
}

function testRejectPromiseThenFinally(): int {
    globalTest = new Test();
    let fixture = new PromiseChainFixture();
    let p = Promise.reject<Object>(new Object());
    // TODO(audovichenko): Remove p1 and p2 variables
    let p1: Promise<Object | null> = p.then<Object | null>((value: Object): Object | null => {
        globalTest!.fail();
        return null;
    }, (error: Object | null): Object | null => {
        if (fixture.state == 0) {
            fixture.state = 1;
        } else {
            globalTest!.fail();
        }
        return null;
    });
    p1.finally(() => {
        if (fixture.state == 1) {
            globalTest!.success();
        } else {
            globalTest!.fail();
        }
    });
    return 0;
}

function testReturnPromise(): Promise<Object> {
    return new Promise<Object>((resolve: (value: Object) => void): void => {
        resolve("Panda");
    });
}

async function testReturnPromiseFromAsync(): Promise<Object> {
    return new Promise<Object>((resolve: (value: Object) => void): void => {
        resolve("Panda");
    });
}

let resolvePromiseFn: ((value: Object) => void) | null = null;

function testReturnPendingPromise(): Promise<Object> {
    return new Promise<Object>((resolve: (value: Object) => void): void => {
        resolvePromiseFn = resolve;
    });
}

function resolvePendingPromise(): boolean {
    if (resolvePromiseFn == null) {
        return false;
    }
    resolvePromiseFn("Panda");
    return true;
}

let unresolved1: Object | null = null;
let unresolved2: Object | null = null;

async function asyncFuncAwait(): Promise<Object | null> {
    let promise: Promise<Object> = new Promise<Object>((resolve: (obj: Object) => void): void => {
        resolve("resolved");
    });
    unresolved1 = await promise;
    return null;
}

let asyncLambdaAwait: () => Promise<Object | null> = async (): Promise<Object | null> => {
    let promise: Promise<Object> = new Promise<Object>((resolve: (obj: Object) => void): void => {
        resolve("resolved");
    });
    unresolved2 = await promise;
    return null;
}

function testAwaitPromise(): int {
    globalTest = new Test();
    let p1: Promise<Object | null> = asyncFuncAwait();
    let p2: Promise<Object | null> = asyncLambdaAwait();
    if (unresolved1 != "resolved" || unresolved2 != "resolved") {
        globalTest!.fail();
    }
    globalTest!.success();
    return 0;
}

function makeResolvedPromise(v: String): Promise<Object> {
    return new Promise<Object>((resolve: (value: Object) => void): void => {
        resolve(v);
    });
}

async function testAwaitJsPromise(pendingP: Promise<Object>): Promise<Object> {
    globalTest = new Test();

    globalTest!.msg("testAwaitJsPromise: start", Test.INFO);
    let res: String = (await pendingP) as String;

    globalTest!.msg("testAwaitJsPromise: await result is: " + res, Test.INFO);

    if (res != "JS_PROMISE_RESULT") {
        globalTest!.msg("testAwaitJsPromise: await result is wrong!", Test.FATAL);
        globalTest!.fail();
    } else {
        globalTest!.msg("testAwaitJsPromise: await result is CORRECT!", Test.INFO);
        globalTest!.success();
    }

    globalTest!.msg("testAwaitJsPromise: done", Test.INFO);
    /**
     * TODO(konstanting, #I67QXC): add a test where we would return here a PENDING promise.
     * Currently this leads to a crash, because:
     *     - testAwaitJsPromise$asyncImpl resolves its linked promise (P1) with makeResolvedPromise's result (P2)
     *     - since we returned P1 to JS as PENDING (due to await presence), it has listeners
     *     - thus, P1 resolution triggers PandaEtsVM::FirePromiseStateChanged, which acquires promise_listeners_lock_
     *     - this leads to wrapping P1's resolution value (P2) into a JS promise
     *     - and since P2 is still PENDING, we must add a listener to it in order to notify JS of its resolution later on
     *     - this triggers PandaEtsVM::AddPromiseListener, which acquires already acquired promise_listeners_lock_ within
     *       the same thread
     *     - BOOM! Assertion fires.
     */
    return makeResolvedPromise("Panda");
}

async function testAsyncBoolean(): Promise<boolean> {
    return true;
}

async function testAsyncByte(): Promise<byte> {
    let x: byte = 127;
    return x;
}

async function testAsyncChar(): Promise<char> {
    return c'x';
}

async function testAsyncShort(): Promise<short> {
    let x: short = 32767;
    return x;
}

async function testAsyncInt(): Promise<int> {
    return 5;
}

async function testAsyncLong(): Promise<long> {
    return Long.MAX_VALUE;
}

async function testAsyncFloat(): Promise<float> {
    let x: float = 3.14;
    return x;
}

async function testAsyncDouble(): Promise<double> {
    return 3.2;
}

async function testAsyncString(): Promise<String> {
    return "Panda";
}

async function testAsyncVoid(): Promise<void> { return undefined }
async function testAsyncVoidEmpty(): Promise<void> {}
async function testAsyncVoidNothing() {}

function testPromiseAllEmpty(): Promise<Object | null> {
    globalTest = new Test();
    let promises: Promise<Object>[] = [];
    return Promise.all(promises).then<Object | null>((v: Object): Object|null => {
        let values: Object[] = v as Object[];
        if (values == null || values.length != 0) {
            globalTest!.fail();
        } else {
            globalTest!.success();
        }
    });
}

function testPromiseAllResolved(): Promise<Object|null> {
    globalTest = new Test();
    let p1: Promise<Object> = Promise.resolve<Object>("abc");
    let p2: Promise<Object> = Promise.resolve<Object>("def");
    let p3: Promise<Object> = Promise.resolve<Object>("xyz");
    return Promise.all([
        p1, p2, p3
    ]).then<Object | null>((v: Object): Object|null => {
        let values: Object[] = v as Object[];
        if (values == null || values.length != 3) {
            globalTest!.fail();
        }
        if (values[0] != "abc") {
            globalTest!.fail();
        }
        if (values[1] != "def") {
            globalTest!.fail();
        }
        if (values[2] != "xyz") {
            globalTest!.fail();
        }
        globalTest!.success();
        return null;
    });
}

function testPromiseAllRejected(): Promise<Object|null> {
    globalTest = new Test();
    let p1: Promise<Object> = Promise.resolve<Object>("abc");
    let p2: Promise<Object> = Promise.reject<Object>("def");
    let p3: Promise<Object> = Promise.resolve<Object>("xyz");
    return Promise.all([
        p1, p2, p3
    ]).then<Object|null>((v: Object): Object|null => {
        globalTest!.fail();
        return null;
    }, (err: Object|null): Object|null => {
        if (err != "def") {
            globalTest!.fail();
        } else {
            globalTest!.success();
        }
        return null;
    });
}

function testPromiseAllSettled(): Promise<Object|null> {
    globalTest = new Test();
    let p1: Promise<Object> = Promise.resolve<Object>("abc");
    let p2: Promise<Object> = Promise.reject<Object>("def");
    let p3: Promise<Object> = Promise.resolve<Object>("xyz");
    return Promise.allSettled([
        p1, p2, p3
    ]).then<Object|null>((v: Object): Object|null => {
        let values = v as Object[];
        if (values == null || values.length != 3) {
            globalTest!.fail();
        }
        if (values[0] != "abc") {
            globalTest!.fail();
        }
        if (values[1] != "def") {
            globalTest!.fail();
        }
        if (values[2] != "xyz") {
            globalTest!.fail();
        }
        globalTest!.success();
        return null;
    }, (err: Object|null): Object|null => {
        globalTest!.fail();
        return null;
    });
}

function testPromiseAnyEmpty(): Promise<Object|null> {
    globalTest = new Test();
    let promises: Promise<Object>[] = [];
    return Promise.any(promises).then<Object|null>((v: Object): Object|null => {
        globalTest!.fail();
    }, (err: Object|null): Object|null => {
        if (err != null && err instanceof AggregateError) {
            globalTest!.success();
        } else {
            globalTest!.fail();
        }
    });
}

function testPromiseAnyResolved(): Promise<Object|null> {
    globalTest = new Test();
    let p1: Promise<Object> = Promise.reject<Object>("abc");
    let p2: Promise<Object> = Promise.reject<Object>("def");
    let p3: Promise<Object> = Promise.resolve<Object>("xyz");
    return Promise.any([
        p1, p2, p3
    ]).then<Object|null>((value: Object): Object|null => {
        if (value != "xyz") {
            globalTest!.fail();
        }
        globalTest!.success();
        return null;
    });
}

function testPromiseAnyRejected(): Promise<Object|null> {
    globalTest = new Test();
    let p1: Promise<Object> = Promise.reject<Object>("abc");
    let p2: Promise<Object> = Promise.reject<Object>("def");
    let p3: Promise<Object> = Promise.reject<Object>("xyz");
    return Promise.any([
        p1, p2, p3
    ]).then<Object|null>((v: Object): Object|null => {
        globalTest!.fail();
        return null;
    }, (err: Object|null): Object|null => {
        if (err == null || !(err instanceof AggregateError)) {
            globalTest!.fail();
        } else {
            globalTest!.success();
        }
        return null;
    });
}

function testPromiseRaceResolved(): Promise<Object|null> {
    globalTest = new Test();
    let p1: Promise<Object> = Promise.resolve<Object>("abc");
    let p2: Promise<Object> = Promise.reject<Object>("def");
    return Promise.race([
        p1, p2
    ]).then<Object|null>((value: Object): Object|null => {
        if (value != "abc") {
            globalTest!.fail();
        }
        globalTest!.success();
        return null;
    }, (value: Object|null): Object|null => {
        globalTest!.fail();
        return null;
    });
}

function testPromiseRaceRejected(): Promise<Object|null> {
    globalTest = new Test();
    let p1: Promise<Object> = Promise.reject<Object>("abc");
    let p2: Promise<Object> = Promise.resolve<Object>("def");
    return Promise.race([
        p1, p2
    ]).then<Object|null>((value: Object): Object|null => {
        globalTest!.fail();
        return null;
    }, (value: Object|null): Object|null => {
        console.println("UDAV: here " + value);
        if (value != "abc") {
            globalTest!.fail();
        }
        globalTest!.success();
        return null;
    });
}
