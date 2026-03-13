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

async function asyncMutexStressTest() {
    let mutex = new concurrency.stackless.AsyncMutex()
    const tasks_counter = 5;
    const workload = 5;

    let count: int = 0;
    let jobs = Array.create<Job<void> | undefined>(tasks_counter, undefined);
    for (let i = 0; i < tasks_counter; ++i) {
        const id = i.toInt();
        jobs[id] = launch() {
            waitForCompletion(async () => {
            for (let i = 0; i < workload; ++i) {
                await mutex.lock();
                let temp = count;
                ++count;
                Coroutine.Schedule();
                arktest.assertEQ(temp + 1, count);
                mutex.unlock();
            }
            })
        };
    }
    for (let job of jobs) { 
        job!.Await();
    }
    arktest.assertEQ(count, tasks_counter * workload);
}

async function asyncMutexSimpleUnlockTest() {
    let mutex = new concurrency.stackless.AsyncMutex();
    let err = new Object();
    arktest.expectError(() => { mutex.unlock(); }, new IllegalLockStateError());
}

async function asyncMutexHardUnlockTest() {
    let mutex = new concurrency.stackless.AsyncMutex()
    const tasks_counter = 10;

    let jobs = Array.create<Job<void> | undefined>(tasks_counter, undefined);
    for (let i = 0; i < tasks_counter; ++i) {
        const id = i.toInt();
        jobs[id] = launch() {
            waitForCompletion(async () => {
            await mutex.lock();
            mutex.unlock();
            })
        };
    }

    for (let job of jobs) { 
        job!.Await();
    }

    arktest.expectError(() => { mutex.unlock(); }, new IllegalLockStateError());
}

async function asyncMutexExclusiveAcquireTest() {
    let mutex = new concurrency.stackless.AsyncMutex();

    let flag = false;

    const tasks_counter = 5;
    let jobs = Array.create<Job<void> | undefined>(tasks_counter, undefined);
    for (let i = 0; i < tasks_counter; ++i) {
        const id = i.toInt();
        jobs[id] = launch() {
            waitForCompletion(async () => {
            await mutex.lock();
            Coroutine.Schedule();
            arktest.assertEQ(flag, false);
            flag = true;
            Coroutine.Schedule();
            arktest.assertEQ(flag, true);
            flag = false;
            mutex.unlock();
            })
        };
    }
    for (let job of jobs) { 
        job!.Await();
    }
}

function main(): int {
    let asyncMutexTestsuite = new arktest.ArkTestsuite("asyncMutexTestsuite");
    let asyncMutexTestsuiteDisabled = new arktest.ArkTestsuite("asyncMutexTestsuite.DISABLED");

    // NOTE(ypigunova): include testcase, when bug will be fixed #33591
    asyncMutexTestsuiteDisabled.addAsyncTest('asyncMutexStressTest', asyncMutexStressTest);
    asyncMutexTestsuite.addAsyncTest('asyncMutexSimpleUnlockTest', asyncMutexSimpleUnlockTest);
    asyncMutexTestsuiteDisabled.addAsyncTest('asyncMutexHardUnlockTest', asyncMutexHardUnlockTest);
    asyncMutexTestsuiteDisabled.addAsyncTest('asyncMutexExclusiveAcquireTest', asyncMutexExclusiveAcquireTest);
    return asyncMutexTestsuite.run();
}
