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

const BYTE_LENGTH: int = 128;
const NUM_MS: int = 20;

let abint32 = new ArrayBuffer(BYTE_LENGTH);
let int32array = new Int32Array(abint32);

class OSMutex {
    constructor() {
        this.buff = new ArrayBuffer(8);
        this.ta = new Int32Array(this.buff);
        Atomics.store(this.ta, 0, 0)
    }

    Lock() {
        let currVal = Atomics.compareExchange(this.ta, 0, 0, 1)
        if (currVal != 0) {
            currVal = Atomics.exchange(this.ta, 0, 2)
            while (currVal != 0) {
                Atomics.wait(this.ta, 0, 2)
                currVal = Atomics.exchange(this.ta, 0, 2)
            }
        }
    }

    Unlock() {
        let currVal = Atomics.exchange(this.ta, 0, 0)
        if (currVal == 2) {
            Atomics.notify(this.ta, 0, 1)
        }
    }

    private buff: ArrayBuffer;
    private ta: Int32Array;
}

function testOSMutex() {
    const NUM_WORKERS = 3;
    const NUM = 100000 / NUM_WORKERS;
    let m = new OSMutex();
    let counter = 0;
    let work = () => {
        for (let i = 0; i < NUM; i++) {
            m.Lock();
            counter++;
            m.Unlock();
        }
    }
    let workers = new Array<EAWorker>(NUM_WORKERS);
    for (let i = 0; i < NUM_WORKERS; i++) {
        workers[i] = new EAWorker();
        workers[i].start();
    }
    for (let i = 0; i < NUM_WORKERS; i++) {
        workers[i].run<void>(work);
    }
    for (let i = 0; i < NUM_WORKERS; i++) {
        workers[i].quit();
    }
    arktest.assertEQ(counter, NUM * NUM_WORKERS, counter.toString());
}

function testMainDoesntBlock(index: int) {
    () => {
        const ab = new ArrayBuffer(128);
        const ta = new Int32Array(ab);
        arktest.assertEQ(Atomics.load(ta, index), 0);
        Atomics.waitAsync(ta, index, 0);
        Atomics.waitAsync(ta, index, 0, 100);
    }();
    Coroutine.Schedule();
    let gcId = GC.startGC(GC.Cause.FULL);
    GC.waitForFinishGC(gcId);
}

async function testSyncTimedWaitNotifyRace(index: int) {
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    let w = new EAWorker();
    w.start();

    for (let i = 0; i < NUM_MS; i++) {
        let jb = w.run<string>((): string => {
            let str: string;
            if (i % 2 == 0) {
                str = Atomics.wait(int32array, index, i, NUM_MS / 2);
            } else {
                str = Atomics.wait(int32array, index, i);
            }
            return str;
        });

        let event = new CompletableJob<void>();
        let k = i;
        setTimeout(() => {
            let l = k;
            launch() {
                Atomics.store(int32array, index, l + 1);
                Atomics.notify(int32array, index);
                event.finish();
            }
        }, i);

        let s = jb.Await();
        event.Await();
        arktest.assertTrue(s == 'timed-out' || s == 'ok' || s == 'not-equal', s);
    }

    w.join();
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_ANY);
}

async function testAsyncTimedWaitNotifyRace(index: int) {
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    arktest.assertTrue(CoroutineExtras.isMainWorker());

    for (let i = 0; i < NUM_MS; i++) {
        let p: Promise<string>;
        if (i % 2 == 0) {
            p = Atomics.waitAsync(int32array, index, i, NUM_MS / 2);
        } else {
            p = Atomics.waitAsync(int32array, index, i);
        }
        let k = i;
        let event = new CompletableJob<void>();
        setTimeout(() => {
            let l = k;
            launch() {
                Atomics.store(int32array, index, l + 1);
                Atomics.notify(int32array, index);
                event.finish();
            }
        }, i);
        let s = await p;
        event.Await();
        arktest.assertTrue(s == 'timed-out' || s == 'ok', s);
    }

    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_ANY);
}

function Spam(ta: Int32Array, index: int, num: int) {
    for (let i = 0; i < num; ++i) {
        let p = Atomics.waitAsync(ta, index, 0);
    }
}

function testWithGC(index: int) {
    const NUM = 500;
    Atomics.store(int32array, index, 0);
    Spam(int32array, index, NUM);
    let gcId = GC.startGC(GC.Cause.FULL);
    GC.waitForFinishGC(gcId);
    let count = Atomics.notify(int32array, index, NUM);
    arktest.assertTrue(count <= NUM, count.toString());
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.await_migrate');
    testSuite.addTest('testOSMutex', testOSMutex);
    testSuite.addTest('testWithGC', () => { return testWithGC(3) });
    testSuite.addTest('testMainDoesntBlock', () => { return testMainDoesntBlock(2) });
    testSuite.addAsyncTest('testSyncTimedWaitNotifyRace', () => { return testSyncTimedWaitNotifyRace(1) });
    testSuite.addAsyncTest('testAsyncTimedWaitNotifyRace', () => { return testAsyncTimedWaitNotifyRace(0) });
    let res = testSuite.run();
    return res;
}
