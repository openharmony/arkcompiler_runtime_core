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
        this.mutex_ = new concurrency.stackless.AsyncMutex();
        this.flagCV_ = new concurrency.stackless.AsyncCondVar();
        this.flag_ = false;
    }
    public async Wait() {
        await this.mutex_.lock();
        if (!this.flag_) {
            await this.flagCV_.wait(this.mutex_);
        }
        this.mutex_.unlock();
    }

    public async SetOne() {
        await this.mutex_.lock();
        this.flagCV_.notifyOne(this.mutex_);
        this.flag_ = true;
        this.mutex_.unlock();
    }

    public async SetAll() {
        await this.mutex_.lock();
        this.flagCV_.notifyAll(this.mutex_);
        this.flag_ = true;
        this.mutex_.unlock();
    }

    public async ResetFlag() {
        await this.mutex_.lock();
        this.flag_ = false;
        this.mutex_.unlock();
    }
    private mutex_: concurrency.stackless.AsyncMutex;
    private flagCV_: concurrency.stackless.AsyncCondVar;
    private flag_: boolean;
}

async function asyncCondVarNotifyOneTest() {
    let event = new Event();
    for (let i = 0; i < 3; ++i) {
        event.ResetFlag();
        let afterWait = new AtomicBoolean(false);
        let afterSet = new AtomicBoolean(false);
        let job1 = launch() {
            await event.Wait();
            arktest.assertTrue(afterSet.load());
            afterWait.store(true);
        };
        let job2 = launch() {
            Coroutine.Schedule();
            arktest.assertFalse(afterWait.load());
            afterSet.store(true);
            await event.SetOne();
        };
        job1.Await();
        job2.Await();
        arktest.assertTrue(afterWait.load());
    }
}

async function asyncCondVarNotifyAllTest() {
    let event = new Event();
    for (let i = 0; i < 3; ++i) {
        event.ResetFlag();
        let counter = new AtomicInt(0);
        const tasks_counter = 10;
        let jobs = new Array<Job<void>>(tasks_counter);
        for (let i = 0; i < tasks_counter; ++i) {
            const id = i.toInt();
            jobs[id] = launch() {
                await event.Wait();
                counter.fetchAdd(1);
            };
        }
        arktest.assertEQ(counter.load(), 0);
        let setAll = launch() {
            Coroutine.Schedule();
            await event.SetAll();
        };
        setAll.Await();
        for (let job of jobs) { 
           job.Await();
        }
        arktest.assertEQ(counter.load(), tasks_counter);
    }
}

async function asyncCondVarTestNotifyWithoutWaiters() {
    let workload = 1000;
    let mutex_ = new concurrency.stackless.AsyncMutex();
    let flagCV_ = new concurrency.stackless.AsyncCondVar();
    launch() {
        for (let i = 0; i < workload; ++i) {
            await mutex_.lock();
            flagCV_.notifyOne(mutex_);
            flagCV_.notifyAll(mutex_);
            mutex_.unlock();
        }
    }.Await();
}

function main(): int {
    let asyncCondVarTestsuite = new arktest.ArkTestsuite("asyncCondVarTestsuite");

    asyncCondVarTestsuite.addAsyncTest('asyncCondVarNotifyOneTest', asyncCondVarNotifyOneTest);
    asyncCondVarTestsuite.addAsyncTest('asyncCondVarTestNotifyWithoutWaiters', asyncCondVarTestNotifyWithoutWaiters);
    asyncCondVarTestsuite.addAsyncTest('asyncCondVarNotifyAllTest', asyncCondVarNotifyAllTest);

    return asyncCondVarTestsuite.run();
}
