/*
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

/**
 *  Check if lock returns information about current execution state.
 */
function asyncLockQueryTest(): int {
    let lock = AsyncLock.request("asyncLockQueryTest");
    let promise = lock.lockAsync((): int => {
        let state = lock.query();
        return state.held.length as int;
    });

    let result = await promise;
    if (result == 1) {
        return 0;
    } else {
        StdDebug.Logger.log("FAILED: AsyncLock query test");
        return 1;
    }
}

/**
 *  Check if the query exception causes deadlock.
 */
function asyncLockQueryNonexistentNameTest(): int {
    let result = 0;
    let lockname = "name";
    try {
        AsyncLock.query(lockname);
    } catch (e: Error) {
        if(e.message != "AsyncLock " + lockname + " not found") {
            result += 1;
        }
    }

    AsyncLock.request(lockname);

    if (result != 0) {
        StdDebug.Logger.log("FAILED: AsyncLock query nonexistent name test");
    }

    return result;
}

/**
 *  Check if async lock survives dead lock.
 */
function asyncLockDeadLockTest(): int {
    let lock = new AsyncLock();

    let promise = lock.lockAsync((): int => {
        try {
            await lock.lockAsync((): int => {
                return 0;
            }, AsyncLockMode.EXCLUSIVE, new AsyncLockOptions(false, null, 1));
        } catch (err: Error) {
            return 0;
        }
        StdDebug.Logger.log("FAILED: AsyncLock dead lock test");
        return 1;
    });

    return await promise;
}

/**
*  Check if callback can be interupted because of isAvailable.
*/
function asyncLockAvailableTest(): int {
    let lock = new AsyncLock();
    let options = new AsyncLockOptions(true, null, 0)

    let promise = lock.lockAsync((): int => {
        try {
            await lock.lockAsync((): int => {
                return 0;
            }, AsyncLockMode.EXCLUSIVE, options);
        } catch (err: Error) {
            return 0;
        }
        StdDebug.Logger.log("FAILED: AsyncLock isAvailable test");
        return 1;
    }, AsyncLockMode.EXCLUSIVE, options);

    return await promise;
}

async function testCallbackExclusive(lock: AsyncLock): Promise<int> {

    let promise = lock.lockAsync((): int => {
        let status = lock.query();
        return status.held.length == 1 ? 0 : 1;
    }, AsyncLockMode.EXCLUSIVE);
    return promise;
}

/**
 *  Check if in EXCLUSIVE mode callbacks execute one by one for any caller.
 */
function asyncLockExclusiveModeTest(): int {

    let lock = new AsyncLock();

    let promise1 = testCallbackExclusive(lock);
    let promise2 = testCallbackExclusive(lock);
    let promise3 = testCallbackExclusive(lock);
    let promise4 = testCallbackExclusive(lock);

    let result = 0;
    result += await promise4;
    result += await promise3;
    result += await promise2;
    result += await promise1;

    if (result != 0) {
        StdDebug.Logger.log("FAILED: AsyncLock EXCLUSIVE mode test");
    }

    return result;
}

let asyncLockAppTestsuite = new ArkTestsuite("asyncLockAppTestsuite");

asyncLockAppTestsuite.addTest("sequentialLockUnlocked", () => {
    assertTrue(sequentialLockUnlocked());
});

asyncLockAppTestsuite.addTest("disarmTimeoutTest", () => {
    assertTrue(disarmTimeoutTest());
});

async function applyLockJob(lockName: string, mode: AsyncLockMode, tidBuf: Uint32Array, tid: number): Promise<void> {
    let lock = AsyncLock.request(lockName);
    lock.lockAsync<void>(async () => {
        let old = Atomics.compareExchange(tidBuf, 0, 0, tid);
        assertEQ(old, 0, "Expected 0, but was: " + old);
        await new Promise<void>((resolve: () => void): void => {setTimeout(resolve, 100)});
        old = Atomics.compareExchange(tidBuf, 0, tid, 0);
        assertEQ(old, tid, "Expected old=" + tid + ", but was: " + old);
    }, mode);
}

function check<T>(p: Promise<T>, currentTest: string): boolean {
    try {
        await p;
        return true;
    } catch (e: Error) {
        return false;
    }
}

function check_negative<T>(p: Promise<T>, currentTest: string): boolean {
    try {
        await p;
        return false;
    } catch (e: Error) {
        return true;
    }
}

function swapChanged(buf: Uint32Array, oldVal: number, newVal: number) {
    assertEQ(buf[0], oldVal, "Test failed");
    buf[0] = newVal;
}

function queryTest2() {
    let lockName = "queryTestLock"
    let lock = AsyncLock.request(lockName);
    let state = AsyncLock.query(lock.name);
    if (!state) {
        throw new Error('Test failed');
    }

    let p = lock.lockAsync<void>(async () => {
        let states: AsyncLockState[] = AsyncLock.queryAll();
        assertNE(states.length, 0, 'Test failed: expected at least 1 state, but was ' + states.length);
        let state = AsyncLock.query(lock.name);
        let pending: AsyncLockInfo[] = state.pending;
        assertEQ(pending.length, 0, 'Test failed: expected pending.length=0, but was ' + pending.length);
        let held: AsyncLockInfo[] = state.held;
        assertEQ(held.length, 1, 'Test failed: expected held.length=1, but was ' + held.length);
        assertNE(held[0].name, lockName, 'Test failed');
    }, AsyncLockMode.EXCLUSIVE);
    return p;
}

const Sleep = (ms: int | undefined): Promise<int> => {
    return new Promise<int>(resolve => {
        setTimeout(() => {
            resolve(1)
        },ms as int)
    });
}

function asynclockStateTest1() {
    let lockName = 'Multiplelocktest1'
    let lock = AsyncLock.request(lockName);
    lock.lockAsync(async() =>{
        await Sleep(1000);
    }, AsyncLockMode.EXCLUSIVE);
    lock.lockAsync(async() => {
    }, AsyncLockMode.SHARED);
    let state = AsyncLock.query(lockName);
    arktest.assertEQ(state.held[0]?.name, lockName, 'Test failed, the state.held[0].name expected Multiplelocktest1, but is ' + state.held[0]?.name);
    arktest.assertEQ(state.held[0]?.mode, AsyncLockMode.EXCLUSIVE, 'Test failed, the state.held[0].mode value expected AsyncLockMode.EXCLUSIVE, but is ' + state.held[0]?.mode);
    let pending : AsyncLockInfo[] = state.pending;
    arktest.assertEQ(pending[0]?.name, lockName, 'Test failed, the pending[0].name expected Multiplelocktest1, but is ' + pending[0]?.name);
    arktest.assertEQ(pending[0]?.mode, AsyncLockMode.SHARED, 'Test failed, the pending[0].mode expected AsyncLockMode.SHARED,but is ' + pending[0]?.mode);
    let res = false;
    for(let i = 0; i < pending.length; i++){
        if(pending[i].name == lockName){
            res = pending[i].mode == AsyncLockMode.SHARED;
        }
    }
    arktest.assertTrue(res,'Test failed, the res expected true.');
}

function asynclockStateTest2() {
    let lockName = 'Multiplelocktest2'
    let lock = AsyncLock.request(lockName);
    lock.lockAsync(async() =>{
        await Sleep(1000);
    }, AsyncLockMode.SHARED);
    lock.lockAsync(async() => {
    }, AsyncLockMode.SHARED);
    let state = AsyncLock.query(lockName);
    arktest.assertEQ(state.held[0]?.name, lockName, 'Test failed, the state.held[0].name expected Multiplelocktest2, but is ' + state.held[0]?.name);
    arktest.assertEQ(state.held[0]?.mode, AsyncLockMode.SHARED, 'Test failed, the state.held[0].mode value expected AsyncLockMode.SHARED, but is ' + state.held[0]?.mode);
    let pending : AsyncLockInfo[] = state.pending;
    arktest.assertEQ(pending.length, 0, 'Test failed, the pending.length expected 0, but is ' + pending.length);
}

function asynclockStateTest3() {
    let lockName = 'Multiplelocktest3'
    let lock = AsyncLock.request(lockName);
    lock.lockAsync(async() =>{
        await Sleep(1000);
    }, AsyncLockMode.SHARED);
    lock.lockAsync(async() => {
    }, AsyncLockMode.EXCLUSIVE);
    let state = AsyncLock.query(lockName);
    arktest.assertEQ(state.held[0]?.name, lockName, 'Test failed, the state.held[0].name expected Multiplelocktest3, but is ' + state.held[0]?.name);
    arktest.assertEQ(state.held[0]?.mode, AsyncLockMode.SHARED, 'Test failed, the state.held[0].mode value expected AsyncLockMode.SHARED, but is ' + state.held[0]?.mode);
    let pending : AsyncLockInfo[] = state.pending;
    arktest.assertEQ(pending[0]?.name, lockName, 'Test failed, the pending[0].name expected Multiplelocktest3, but is ' + pending[0]?.name);
    arktest.assertEQ(pending[0]?.mode, AsyncLockMode.EXCLUSIVE, 'Test failed, the pending[0].mode expected AsyncLockMode.EXCLUSIVE,but is ' + pending[0]?.mode);
    let res = false;
    for(let i = 0; i < pending.length; i++){
    if(pending[i].name == lockName){
            res = pending[i].mode == AsyncLockMode.EXCLUSIVE;
        }
    }
    arktest.assertTrue(res,'Test failed, the res expected true.');
}

function asynclockStateTest4() {
    let lockName = 'Multiplelocktest4'
    let lock = AsyncLock.request(lockName);
    lock.lockAsync(async() =>{
        await Sleep(1000);
    }, AsyncLockMode.EXCLUSIVE);
    lock.lockAsync(async() => {
    }, AsyncLockMode.EXCLUSIVE);
    let state = AsyncLock.query(lockName);
    arktest.assertEQ(state.held[0]?.name, lockName, 'Test failed, the state.held[0].name expected Multiplelocktest4, but is ' + state.held[0]?.name);
    arktest.assertEQ(state.held[0]?.mode, AsyncLockMode.EXCLUSIVE, 'Test failed, the state.held[0].mode value expected AsyncLockMode.EXCLUSIVE, but is ' + state.held[0]?.mode);
    let pending : AsyncLockInfo[] = state.pending;
    arktest.assertEQ(pending[0]?.name, lockName, 'Test failed, the pending[0].name expected Multiplelocktest4, but is ' + pending[0]?.name);
    arktest.assertEQ(pending[0]?.mode, AsyncLockMode.EXCLUSIVE, 'Test failed, the pending[0].mode expected AsyncLockMode.EXCLUSIVE,but is ' + pending[0]?.mode);
    let res = false;
    for(let i = 0; i < pending.length; i++){
        if(pending[i].name == lockName){
            res = pending[i].mode == AsyncLockMode.EXCLUSIVE;
        }
    }
    arktest.assertTrue(res,'Test failed, the res expected true.');
}

function assertasynclockStateTest1() {
    arktest.expectNoThrow(() => { asynclockStateTest1(); });
}

function assertasynclockStateTest2() {
    arktest.expectNoThrow(() => { asynclockStateTest2(); });
}

function assertasynclockStateTest3() {
    arktest.expectNoThrow(() => { asynclockStateTest3(); });
}

function assertasynclockStateTest4() {
    arktest.expectNoThrow(() => { asynclockStateTest4(); });
}

function abortTest() {
    let lock = new AsyncLock();
    let s: AbortSignal<string> = {aborted: false, reason: "Aborted"};
    let options = new AsyncLockOptions<string>();
    options.isAvailable = false;
    options.signal = s;
    let p: Promise<number|string> = lock.lockAsync<number, string>(() => {
        return 5;
    }, AsyncLockMode.EXCLUSIVE, options);
    s.aborted = true;
    let p1 = p.then(
        (v: Double) => {
            throw new Error('Test failed. Promise is fulfilled.')
        },
        (err: Error): void => {
             arktest.assertEQ(err.message, "Aborted", 'Test failed. Wrong error value.');
    }
    );
    return check(p1, "abortTest");
}

function deadlockTest() {
    let lock = new AsyncLock();
    let options = new AsyncLockOptions<string>();
    options.timeout = 1000;
    let p: Promise<number|string> = lock.lockAsync<number, string>(async (): Promise<number> => {
        let options = new AsyncLockOptions<string>();
        options.timeout = 1000;
        await lock.lockAsync(() => {
            throw new Error('Test failed: inner lock acquired.');
        }, AsyncLockMode.EXCLUSIVE, options);
        return 0 as number;
    }, AsyncLockMode.EXCLUSIVE, options);
    // setTimeout is needed to force event loop to process AsyncLock's timer
    setTimeout(() => {
    }, (options.timeout as int));
    return check_negative(p, "deadlockTest");
}

function nestedDeadlockTest() {
    let options = new AsyncLockOptions<void>();
    options.timeout = 1000;

    let lockA = new AsyncLock();
    let p = lockA.lockAsync<void, void>(async () => {
        let lockB = new AsyncLock();
        await lockB.lockAsync<void, void>(async () => {
            await lockA.lockAsync<void,void>(() => {
                throw new Error('Test failed: acquired the inner lock A.');
            }, AsyncLockMode.EXCLUSIVE, options);
        }, AsyncLockMode.EXCLUSIVE, options);
    }, AsyncLockMode.EXCLUSIVE, options);
    // setTimeout is needed to force event loop to process AsyncLock's timer
    setTimeout(() => {
    }, (options.timeout as int));
    return check_negative(p, "nestedDeadlockTest");
}

function twoThreadedDeadlockT2(lockNameL1: string, lockNameL2: string, flagBuf: Uint32Array) {
    let L2 = AsyncLock.request(lockNameL2);
    L2.lockAsync<void>(async () => { // F2
        Atomics.compareExchange(flagBuf, 0, 0, 1);
        // now lock the second lock
        let L1 = AsyncLock.request(lockNameL1);
        await L1.lockAsync<void>(() => {
          // deadlock should happen, then T1 timeout exceeds and utils.locks will unlock
        }, AsyncLockMode.EXCLUSIVE);
    }, AsyncLockMode.EXCLUSIVE);
}

function twoLocksSingleTest() {
    let sab = new ArrayBuffer(4);
    let flagBuf = new Uint32Array(sab);
    flagBuf[0] = 0;
    let lock1 = new AsyncLock();
    let lock2 = new AsyncLock();

    let p1 = lock1.lockAsync<void>(async () => {
        swapChanged(flagBuf, 0, 1);
        await Promise.resolve();
        swapChanged(flagBuf, 1, 0);
    }, AsyncLockMode.EXCLUSIVE);
    let p2 = lock2.lockAsync<void>(async () => {
        swapChanged(flagBuf, 0, 2);
        await Promise.resolve();
        swapChanged(flagBuf, 2, 0);
    }, AsyncLockMode.EXCLUSIVE);
    return check(Promise.all([p1, p2]), "twoLocksSingleTest");
}

async function exclusiveLockAcquireJob(): Promise<void> {
    let lock = new AsyncLock();
    lock.lockAsync<void>(async () => {
    }, AsyncLockMode.EXCLUSIVE);
}

function twoThreadsAcquireOwnLocks() {
    let p1: Promise<Object> = taskpool.execute(exclusiveLockAcquireJob) as Promise<Object>;
    let p2: Promise<Object> = taskpool.execute(exclusiveLockAcquireJob) as Promise<Object>;
    return check(Promise.all([p1, p2]), "twoThreadsAcquireOwnLocks");
}

async function lockJob(lockName: string, flagBuf: Uint32Array): Promise<void> {
    let lock = AsyncLock.request(lockName);
    lock.lockAsync<void>(async () => {
        Atomics.compareExchange(flagBuf, 0, 0, 1);
        await new Promise<void>((resolve: () => void): void => {setTimeout(resolve, 100)});
        Atomics.compareExchange(flagBuf, 0, 1, 0);
    }, AsyncLockMode.SHARED);
}

async function multipleWakeupTest() {
    let lockName = "multipleWakeupLock";
    let sab = new ArrayBuffer(4);
    let flagBuf = new Uint32Array(sab);
    flagBuf[0] = 0;
    let p: Promise<Object> = taskpool.execute(lockJob, lockName, flagBuf) as Promise<Object>;
    // busy loop
    while (Atomics.load(flagBuf, 0) == 0) {
    }
    let count = 10;
    let promises = new Array<Promise<void>>(count);
    let lock = AsyncLock.request(lockName);
    for (let i = 0; i < count; ++i) {
        promises[i] = lock.lockAsync<void>(async () => {
        }, AsyncLockMode.SHARED);
    }
    await p;
    return check(Promise.all(promises), "multipleWakeupTest");
}

async function waitForLockBody() {
    let sab = new ArrayBuffer(4);
    let flagBuf = new Uint32Array(sab);
    flagBuf[0] = 0;
    let lock = new AsyncLock();

    await lock.lockAsync<void>(async () => {
        swapChanged(flagBuf, 0, 1);
    }, AsyncLockMode.EXCLUSIVE);
    assertEQ(flagBuf[0], 1, "Test failed");
}

function waitForLock() {
    return check(waitForLockBody(), "waitForLock");
}

function dontWaitForLock() {
    let sab = new ArrayBuffer(4);
    let flagBuf = new Uint32Array(sab);
    flagBuf[0] = 0;
    let lock = new AsyncLock();
    let p = lock.lockAsync<void>(async () => {
        swapChanged(flagBuf, 0, 1)
    }, AsyncLockMode.EXCLUSIVE);
    assertEQ(flagBuf[0], 0, "Test failed");
    return check(p, "dontWaitForLock");
}

function queryWithDeadlockTest() {
    let options = new AsyncLockOptions<string>();
    options.timeout = 1000;
    let lock = new AsyncLock();
    let p = lock.lockAsync<void>(async () => {
        await lock.lockAsync(() => {
        }, AsyncLockMode.EXCLUSIVE, options);
    }, AsyncLockMode.EXCLUSIVE);

    setTimeout(() => {
        let states: AsyncLockState[] = AsyncLock.queryAll();
    }, 100);

    return check_negative(p, "queryWithDeadlockTest");
}

async function lockHolderJob(lockName: string, timeout: number, flagBuf: Uint32Array): Promise<void> {
    let lock = AsyncLock.request(lockName);
    lock.lockAsync<void>(async () => {
        Atomics.compareExchange(flagBuf, 0, 0, 1);
        await new Promise<void>((resolve: () => void): void => {setTimeout(resolve, 100)});
    }, AsyncLockMode.EXCLUSIVE);
}

function sequentialLock() {
    let lockNameB = "sequentialLockB";
    let sab = new ArrayBuffer(4);
    let flagBuf = new Uint32Array(sab);
    flagBuf[0] = 0;
    // NOTE(mredkin): #23718 signatures needed
    // taskpool.execute(lockHolderJob, lockNameB, 100, flagBuf);
    let p = new Promise<void>((resolve: () => void, reject: (e: Error) => void): void => {
        let lockA = new AsyncLock();
        lockA.lockAsync(() => {
            // busy loop
            while (Atomics.load(flagBuf, 0) != 1) {
            }
            let lockB = AsyncLock.request(lockNameB);
            lockB.lockAsync(() => {
                let old = Atomics.load(flagBuf, 0);
                if (old == 2) {
                    resolve();
                } else {
                    reject(new Error("Expected 2, but was: " + old));
                }
            });
            let old = Atomics.compareExchange(flagBuf, 0, 1, 2);
            if (old != 1) {
              reject(new Error("Expected 1, but was: " + old));
            }
        });
    });
    return check(p, "sequentialLock");
}

function sequentialLockInf() {
    let lockNameB = "sequentialLockInfB";
    let sab = new ArrayBuffer(4);
    let flagBuf = new Uint32Array(sab);
    flagBuf[0] = 0;
    // NOTE(mredkin): #23718 signatures needed
    // taskpool.execute(lockHolderJob, lockNameB, 1000, flagBuf);
    let p = new Promise<void>((resolve: () => void, reject: (e: Error) => void): void => {
        let lockA = new AsyncLock();
        lockA.lockAsync(() => {
            // busy loop
            while (Atomics.load(flagBuf, 0) != 1) {
            }
            let lockB = AsyncLock.request(lockNameB);
            lockB.lockAsync(() => {
                let old = Atomics.compareExchange(flagBuf, 0, 2, 3);
                if (old != 2) {
                    reject(new Error("Expected 2, but was: " + old));
                }
            });
            let old = Atomics.compareExchange(flagBuf, 0, 1, 2);
            if (old == 1) {
                resolve();
            } else {
                reject(new Error("Expected 1, but was: " + old));
            }
        });
    });
    return check(p, "sequentialLockInf");
}

function sequentialLockUnlocked() {
    let sab = new ArrayBuffer(4);
    let flagBuf = new Uint32Array(sab);
    flagBuf[0] = 0;
    let lockA = new AsyncLock();
    let p = lockA.lockAsync<void>(async () => {
        swapChanged(flagBuf, 0, 1);
        let lockB = new AsyncLock();
        await lockB.lockAsync(() => {
            swapChanged(flagBuf, 1, 2);
        });
        swapChanged(flagBuf, 2, 3);
    });
    return check(p, "sequentialLockUnlocked");
}

function sequentialLockUnlocked1() {
    let p = new Promise<void>((resolve: () => void, reject: (e: Error) => void): void => {
        let lockA = new AsyncLock();
        lockA.lockAsync<void>(/*F1*/ async () => {
            let lockB = new AsyncLock();
            let p = lockB.lockAsync<void>(/*F2*/ async (): Promise<void> => {
                let res = await lockA.lockAsync<number>(/*F3*/ (): number => {
                    return 42.0
                }, AsyncLockMode.SHARED);
                if (res == 42) {
                    resolve();
                } else {
                    reject(new Error("Test failed. Expected 42, but was " + res));
                }
            });
        });
    });
    return check(p, "sequentialLockUnlocked1");
}

function sequentialLockUnlocked3() {
    let p = new Promise<void>((resolve: () => void, reject: (e: Error) => void): void => {
        let lockA = new AsyncLock();
        lockA.lockAsync<void>(/*F1*/ async () => {
            let lockB = new AsyncLock();
            lockB.lockAsync<void>(/*F2*/ async () => {
                let p1 = lockA.lockAsync<number>(/*F3*/ () => {
                    return 42
                }, AsyncLockMode.EXCLUSIVE);
                p1.then((v: int): void => {
                    if (v == 42) {
                        resolve();
                    } else {
                        reject(new Error("Expected 42, but was " + v));
                    }
                }, (p1: Error): void => {});
            });
        });
    });
    return check(p, "sequentialLockUnlocked3");
}

function disarmTimeoutTest() {
    let lock = new AsyncLock();
    let options = new AsyncLockOptions<void>();
    options.timeout = 1000;
    let p: Promise<void> = lock.lockAsync<void, void>(() => {
    }, AsyncLockMode.EXCLUSIVE, options);
    return check(p, "disarmTimeoutTest");
}

function disarmTimeoutLongAwaitTest() {
    /*
    * Create an exclusive lock and set the timeout period to 1 second.
    * In the callback function, await 5 seconds is used to verify that the
    * callback function can be properly executed and ends after 5 seconds instead of rejecting after 1 second.
    * */
    let lock = new AsyncLock();
    let options = new AsyncLockOptions<void>();
    options.timeout = 500;
    let p: Promise<void> = lock.lockAsync<void, void>(async () => {
        await new Promise<number>((resolve: (a: int) => void, reject: (e: Error) => void): void => {
            setTimeout(() => {
                resolve(123);
            }, 1500);
        });
    }, AsyncLockMode.EXCLUSIVE, options);
    return check(p, "disarmTimeoutLongAwaitTest");
}

function acquireLockTwiceSuccess(outerMode: AsyncLockMode, innerMode: AsyncLockMode,
                                 isAvailable: boolean): Promise<void> {
    let lock = new AsyncLock();
    return new Promise<void>((resolve: () => void): void => {
        let innerOptions = new AsyncLockOptions<void>();
        innerOptions.isAvailable = isAvailable;
        lock.lockAsync<void>(() => {
            lock.lockAsync<void, void>(() => {
                resolve();
            }, innerMode, innerOptions);
        }, outerMode);
    });
}

function acquireLockTwiceSharedSharedAvailable() {
    return check(acquireLockTwiceSuccess(AsyncLockMode.SHARED, AsyncLockMode.SHARED, true), "acquireLockTwiceSharedSharedAvailable");
}

function acquireLockTwiceSharedSharedNotAvailable() {
    return check(acquireLockTwiceSuccess(AsyncLockMode.SHARED, AsyncLockMode.SHARED, false), "acquireLockTwiceSharedSharedNotAvailable");
}

function acquireLockTwiceReject(outerMode: AsyncLockMode, innerMode: AsyncLockMode,
                                isAvailable: boolean): Promise<void> {
    let lock = new AsyncLock();
    return new Promise<void>((resolve: () => void, reject: (e: Error) => void): void => {
        let innerOptions = new AsyncLockOptions<void>();
        innerOptions.isAvailable = isAvailable;
        lock.lockAsync((): void => {
            lock.lockAsync(() => {
            }, innerMode, innerOptions).then((): void => {
                reject(new Error("Test failed"));
            }, (e: Error): void => {
                resolve()
            });
        }, outerMode);
    });
}

function acquireLockTwiceSharedExclusiveAvailable() {
    return check(acquireLockTwiceReject(AsyncLockMode.SHARED, AsyncLockMode.EXCLUSIVE, true), "acquireLockTwiceSharedExclusiveAvailable");
}

function acquireLockTwiceSharedExclusiveNotAvailable() {
    return check(acquireLockTwiceSuccess(AsyncLockMode.SHARED, AsyncLockMode.EXCLUSIVE, false), "acquireLockTwiceSharedExclusiveNotAvailable");
}

function acquireLockTwiceExclusiveSharedAvailable() {
    return check(acquireLockTwiceReject(AsyncLockMode.EXCLUSIVE, AsyncLockMode.SHARED, true), "acquireLockTwiceExclusiveSharedAvailable");
}

function acquireLockTwiceExclusiveSharedNotAvailable() {
    return check(acquireLockTwiceSuccess(AsyncLockMode.EXCLUSIVE, AsyncLockMode.SHARED, false), "acquireLockTwiceExclusiveSharedNotAvailable");
}

function acquireLockTwiceExclusiveExclusiveAvailable() {
    return check(acquireLockTwiceReject(AsyncLockMode.EXCLUSIVE, AsyncLockMode.EXCLUSIVE, true), "acquireLockTwiceExclusiveExclusiveAvailable");
}

function acquireLockTwiceExclusiveExclusiveNotAvailable() {
    return check(acquireLockTwiceSuccess(AsyncLockMode.EXCLUSIVE, AsyncLockMode.EXCLUSIVE, false), "acquireLockTwiceExclusiveExclusiveNotAvailable");
}

async function mainExecutor(index: number, flagBuf: Uint32Array, timeout: number) {
    let lock: AsyncLock = AsyncLock.request("doubleExitSharedLock");
    await lock.lockAsync(async () => {
        flagBuf[index] = 1;
        assertEQ(flagBuf[2], 0, "Unexpected set flag at 2");
        await new Promise<void>((resolve: () => void): void => setTimeout(resolve, timeout as int));
        flagBuf[index] = 0;
    }, AsyncLockMode.SHARED);
}

async function taskExecutor(flagBuf: Uint32Array, timeout: number) {
    let lock: AsyncLock = AsyncLock.request("doubleExitSharedLock");
    await lock.lockAsync(async () => {
        flagBuf[2] = 1;
        assertEQ(flagBuf[0], 0, "Unexpected set flag at 0");
        assertEQ(flagBuf[1], 0, "Unexpected set flag at 1");
        await new Promise<void>((resolve: () => void): void => setTimeout(resolve, timeout as int));
        flagBuf[2] = 0;
    }, AsyncLockMode.EXCLUSIVE);
}

function doubleExitSharedLock() {
    let sab = new ArrayBuffer(3 * 4);
    let flagBuf = new Uint32Array(sab);
    for (let i = 0; i < flagBuf.length; ++i) {
        flagBuf[i] = 0;
    }
    return check(Promise.all([
        mainExecutor(0, flagBuf, 500),
        mainExecutor(1, flagBuf, 300),
        // NOTE(mredkin): #23718 signatures needed
        // taskpool.execute(taskExecutor, flagBuf, 100),
    ]), "doubleExitSharedLock");
}

function main(): int {
    let asyncLockTestsuite = new ArkTestsuite("asyncLockTestsuite");

    asyncLockTestsuite.addTest("TestWithEqualityAndNonEquality", () => {
        let failures = 0;
        failures += asyncLockQueryTest();
        failures += asyncLockDeadLockTest();
        failures += asyncLockAvailableTest();
        failures += asyncLockExclusiveModeTest();
        failures += asyncLockQueryNonexistentNameTest();

        assertEQ(failures, 0);
    });
    asyncLockTestsuite.addTest('asynclockStateTest1', assertasynclockStateTest1);
    asyncLockTestsuite.addTest('asynclockStateTest2', assertasynclockStateTest2);
    asyncLockTestsuite.addTest('asynclockStateTest3', assertasynclockStateTest3);
    asyncLockTestsuite.addTest('asynclockStateTest4', assertasynclockStateTest4);
    asyncLockTestsuite.addTest('asynclockabortTest', () => { arktest.assertTrue(abortTest()); });
    
    let summ = 0;
    summ += asyncLockTestsuite.run();
    summ += asyncLockAppTestsuite.run();
    return summ;
}
