/*
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

class Event {
    constructor() {
        this.promise = new Promise<boolean>((resolve: (val: boolean) => void) => {
            this.resolveFn = resolve;
        })
    }

    public wait(): boolean {
        return this.promise.awaitSync();
    }

    public fire(val: boolean = true) {
        this.resolveFn!(val);
    }

    private promise: Promise<boolean>;
    private resolveFn: ((val: boolean) => void) | null = null;
}

async function asyncRWLockSharedReadTest() {
    let rwLock = new concurrency.stackless.AsyncReadWriteLock();
    let readLock = rwLock.readLock();
    const tasks_counter = 10;

    let sharedValue: int = 42;
    let jobs = Array.create<Promise<void> | undefined>(tasks_counter, undefined);

    // First, acquire read lock in main task
    let acquiredInMain = false;
    let job0 = launch() {
        // NOTE(zhuheng_z, #11066): should be removed after launch will be correctly supported for async functions
        (async () => {
        await readLock.lock();
        acquiredInMain = true;

        for (let i = 0; i < tasks_counter; ++i) {
            const id = i.toInt();
            jobs[id] = launch() {
                // NOTE(zhuheng_z, #11066): should be removed after launch will be correctly supported for async functions
                (async () => {
                await readLock.lock();
                let value = sharedValue;
                Coroutine.Schedule();
                arktest.assertEQ(value, 42);
                readLock.unlock();
                })().awaitSync()
            };
        }

        readLock.unlock();
        })().awaitSync()
    };

    await job0;
    for (let job of jobs) {
        await job!;
    }

    arktest.assertEQ(sharedValue, 42);
}

async function asyncRWLockExclusiveWriteTest() {
    let rwLock = new concurrency.stackless.AsyncReadWriteLock();
    let writeLock = rwLock.writeLock();
    const tasks_counter = 5;
    const workload = 5;

    let count: int = 0;
    let jobs = Array.create<Promise<void> | undefined>(tasks_counter, undefined);

    for (let i = 0; i < tasks_counter; ++i) {
        const id = i.toInt();
        jobs[id] = launch() {
            // NOTE(zhuheng_z, #11066): should be removed after launch will be correctly supported for async functions
            (async () => {
            for (let j = 0; j < workload; ++j) {
                await writeLock.lock();
                let temp = count;
                ++count;
                // Coroutine.Schedule();
                arktest.assertEQ(temp + 1, count);
                writeLock.unlock();
            }
            })().awaitSync()
        };
    }
    for (let job of jobs) {
        await job!;
    }

    arktest.assertEQ(count, tasks_counter * workload);
}

async function asyncRWLockReadWriteMutualExclusionTest() {
    let rwLock = new concurrency.stackless.AsyncReadWriteLock();
    let readLock = rwLock.readLock();
    let writeLock = rwLock.writeLock();

    let writeAcquired = false;
    let writeLockAttempted = false;
    let readLockHeld = false;

    let waiter1 = new Event();
    let waiter = new Event();

    let job0 = launch() {
        // NOTE(zhuheng_z, #11066): should be removed after launch will be correctly supported for async functions
        (async () => {
        await readLock.lock();
        readLockHeld = true;

        let job1 = launch() {
            // NOTE(zhuheng_z, #11066): should be removed after launch will be correctly supported for async functions
            (async () => {
            writeLockAttempted = true;
            waiter1.fire();
            await writeLock.lock();
            writeAcquired = true;
            Coroutine.Schedule();
            writeLock.unlock();
            })().awaitSync()
        };
        waiter1.wait();
        arktest.assertTrue(writeLockAttempted);
        arktest.assertFalse(writeAcquired);

        readLock.unlock();
        await job1;
        arktest.assertTrue(writeAcquired);
        })().awaitSync()
    };

    await job0;
}

async function asyncRWLockIllegalUnlockTest() {
    let rwLock = new concurrency.stackless.AsyncReadWriteLock();
    let readLock = rwLock.readLock();
    let writeLock = rwLock.writeLock();
    let err = new Object();

    // Try to unlock without locking
    try {
        readLock.unlock();
    } catch (e) {
        err = e;
    }
    arktest.assertTrue(err instanceof IllegalLockStateError);

    err = new Object();
    try {
        writeLock.unlock();
    } catch (e) {
        err = e;
    }
    arktest.assertTrue(err instanceof IllegalLockStateError);
}

async function asyncRWLockWriteWriteExclusionTest() {
    let rwLock = new concurrency.stackless.AsyncReadWriteLock();
    let writeLock = rwLock.writeLock();

    let flag = false;
    const tasks_counter = 5;

    let jobs = Array.create<Promise<void> | undefined>(tasks_counter, undefined);
    for (let i = 0; i < tasks_counter; ++i) {
        const id = i.toInt();
        jobs[id] = launch() {
            // NOTE(zhuheng_z, #11066): should be removed after launch will be correctly supported for async functions
            (async () => {
            await writeLock.lock();
            Coroutine.Schedule();
            arktest.assertEQ(flag, false);
            flag = true;
            Coroutine.Schedule();
            arktest.assertEQ(flag, true);
            flag = false;
            writeLock.unlock();
            })().awaitSync()
        };
    }

    for (let job of jobs) {
        await job!;
    }
}

async function asyncRWLockMultipleReadersTest() {
    let rwLock = new concurrency.stackless.AsyncReadWriteLock();
    let readLock = rwLock.readLock();

    const readers_count = 10;
    let activeReaders = Array.create<int>(readers_count, 0);

    let jobs = Array.create<Promise<void> | undefined>(readers_count, undefined);
    for (let i = 0; i < readers_count; ++i) {
        const id = i.toInt();
        jobs[id] = launch() {
            // NOTE(zhuheng_z, #11066): should be removed after launch will be correctly supported for async functions
            (async () => {
            await readLock.lock();
            activeReaders[id] = 1;
            Coroutine.Schedule();

            let total = 0;
            for (let j = 0; j < readers_count; ++j) {
                total += activeReaders[j];
            }
            arktest.assertTrue(total >= 1);

            activeReaders[id] = 0;
            readLock.unlock();
            })().awaitSync()
        };
    }

    for (let job of jobs) {
        await job!;
    }
}

async function asyncRWLockMixedStressTest() {
    let rwLock = new concurrency.stackless.AsyncReadWriteLock();
    let readLock = rwLock.readLock();
    let writeLock = rwLock.writeLock();

    let sharedCounter: int = 0;
    const reader_count = 5;
    const writer_count = 3;
    const workload = 5;

    let total_jobs = reader_count + writer_count;
    let jobs = Array.create<Promise<void> | undefined>(total_jobs, undefined);

    for (let i = 0; i < reader_count; ++i) {
        const id = i.toInt();
        jobs[id] = launch() {
            // NOTE(zhuheng_z, #11066): should be removed after launch will be correctly supported for async functions
            (async () => {
            for (let j = 0; j < workload; ++j) {
                await readLock.lock();
                let temp = sharedCounter;
                Coroutine.Schedule();
                arktest.assertTrue(temp >= 0);
                readLock.unlock();
            }
            })().awaitSync()
        };
    }

    for (let i = 0; i < writer_count; ++i) {
        const id = (reader_count + i).toInt();
        jobs[id] = launch() {
            // NOTE(zhuheng_z, #11066): should be removed after launch will be correctly supported for async functions
            (async () => {
            for (let j = 0; j < workload; ++j) {
                await writeLock.lock();
                let temp = sharedCounter;
                sharedCounter = temp + 1;
                Coroutine.Schedule();
                arktest.assertEQ(sharedCounter, temp + 1);
                writeLock.unlock();
            }
            })().awaitSync()
        };
    }

    for (let job of jobs) {
        await job!;
    }

    arktest.assertEQ(sharedCounter, writer_count * workload);
}

async function main(): Promise<int> {
    let asyncRWLockTestsuite = new arktest.ArkTestsuite("asyncRWLockTestsuite");

    asyncRWLockTestsuite.addAsyncTest('asyncRWLockSharedReadTest', asyncRWLockSharedReadTest);
    asyncRWLockTestsuite.addAsyncTest('asyncRWLockExclusiveWriteTest', asyncRWLockExclusiveWriteTest);
    asyncRWLockTestsuite.addAsyncTest('asyncRWLockReadWriteMutualExclusionTest', asyncRWLockReadWriteMutualExclusionTest);
    asyncRWLockTestsuite.addAsyncTest('asyncRWLockIllegalUnlockTest', asyncRWLockIllegalUnlockTest);
    asyncRWLockTestsuite.addAsyncTest('asyncRWLockWriteWriteExclusionTest', asyncRWLockWriteWriteExclusionTest);
    asyncRWLockTestsuite.addAsyncTest('asyncRWLockMultipleReadersTest', asyncRWLockMultipleReadersTest);
    asyncRWLockTestsuite.addAsyncTest('asyncRWLockMixedStressTest', asyncRWLockMixedStressTest);

    return await asyncRWLockTestsuite.runAsync();
}
