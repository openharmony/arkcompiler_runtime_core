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

type callbackT = (id: int) => void;

let event = new PromiseWithResolvers<void>();

class SharedJob<T> {
    constructor(jobs: Array<T>) {
        this.sharedJob = new PromiseWithResolvers<void>();
        SharedJob.completeJob(jobs, this.sharedJob);
    }

    public Await(): void {
        this.sharedJob.promise.awaitSync();
    }

    private static async completeJob<T>(jobs: Array<T>, sharedJob: PromiseWithResolvers<void>) {
        Coroutine.Schedule();
        for (let job of jobs) {
            if (job instanceof Promise) {
                await job;
            } else if (job instanceof PromiseWithResolvers) {
                await job.promise;
            } else {
                throw new Error('invalid type')
            }
        }
        sharedJob.resolve(undefined);
    }

    private sharedJob: PromiseWithResolvers<void>;
}

function runConcurrently(callback: callbackT, coroutines: number) {
    let jobs = new Array<Promise<void>>();
    for (let i = 0; i < coroutines; ++i) {
        const id = i.toInt();
        jobs.push(launch() {
            return callback(id)
        });
    }
    return new SharedJob<Promise<void>>(jobs);
}

// The test checks exclusive access to the critical section
function exclusiveLockTest(locker: Lock) {
    const coroutines = 10;
    const workload = 1000;

    let count = 0;
    runConcurrently((id: int) => {
        for (let i = 0; i < workload; ++i) {
            locker.lock();
            count++;
            locker.unlock();
        }
    }, coroutines).Await();
    arktest.assertEQ(count, coroutines * workload);
}

// The test checks that there is no deadlock with the ReadLock
function sharedLockTest() {
    const coroutines = 10;

    let rLock = new RWLock().readLock();
    let jobs = new Array<PromiseWithResolvers<void>>(coroutines, (index: int) => new PromiseWithResolvers<void>());
    let sharedJob = new SharedJob<PromiseWithResolvers<void>>(jobs);
    runConcurrently((id: int) => {
        rLock.lock();
        jobs[id].resolve(undefined);
        sharedJob.Await();
        rLock.unlock();
    }, coroutines).Await();
}

// The test checks whether the correct error was thrown
function illegalLockStateTest() {
    let arrError = new Array<Object>();
    let rwLock = new RWLock();
    try {
        rwLock.readLock().lock();
        rwLock.writeLock().unlock();
    } catch (e) {
        rwLock.readLock().unlock();
        arrError.push(e);
    }

    try {
        rwLock.writeLock().lock();
        rwLock.readLock().unlock();
    } catch (e) {
        rwLock.writeLock().unlock();
        arrError.push(e);
    }

    try {
        rwLock.readLock().unlock();
    } catch (e) {
        arrError.push(e);
    }

    try {
        rwLock.writeLock().unlock();
    } catch (e) {
        arrError.push(e);
    }

    try {
        new Mutex().unlock();
    } catch (e) {
        arrError.push(e);
    }

    arktest.assertEQ(arrError.length, 5);
    for (let error of arrError) {
        arktest.assertTrue(error instanceof IllegalLockStateError);
    }
}

// The test checks whether read-write locks are mutually exclusive
async function readWriteMutualExclusionTest(): Promise<void> {
    let rwLock = new RWLock();
    let readLock = rwLock.readLock();
    let writeLock = rwLock.writeLock();

    let writeAcquired = false;
    let writeLockAttempted = false;

    readLock.lock();

    let job = launch() {
        writeLockAttempted = true;
        event.resolve(undefined);
        writeLock.lock();
        writeAcquired = true;
        writeLock.unlock();
    };
    await event.promise;

    arktest.assertTrue(writeLockAttempted);
    arktest.assertFalse(writeAcquired);

    readLock.unlock();
    await job;
    arktest.assertTrue(writeAcquired);
}

function exclusiveMutexLockTest() {
    exclusiveLockTest(new Mutex());
}

function exclusiveRWLockTest() {
    exclusiveLockTest(new RWLock().writeLock());
}

async function main(): Promise<int> {
    let testSuite = new arktest.ArkTestsuite('coroutines.sync_primitives');
    testSuite.addTest('exclusiveMutexLockTest', exclusiveMutexLockTest);
    testSuite.addTest('exclusiveRWLockTest', exclusiveRWLockTest);
    testSuite.addTest('sharedLockTest', sharedLockTest);
    testSuite.addTest('illegalLockStateTest', illegalLockStateTest);
    testSuite.addAsyncTest('readWriteMutualExclusionTest', async () => { await readWriteMutualExclusionTest(); });

    let res = await testSuite.runAsync();
    return res;
}
