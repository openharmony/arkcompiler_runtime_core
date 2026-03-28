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

import { AtomicInt } from 'std/concurrency'

function nonconcurrentLoadStoreTest() {
    let atomicInt = new AtomicInt(1);
    arktest.assertEQ(atomicInt.load(), 1);

    atomicInt.store(2);
    arktest.assertEQ(atomicInt.load(), 2)
}


function nonconcurrentExchangeTest() {
    let atomicInt = new AtomicInt(1);

    let prev = atomicInt.exchange(2);
    arktest.assertEQ(prev, 1);
    arktest.assertEQ(atomicInt.load(), 2);
}

function nonconcurrentCompareAndSwapTest() {
    let atomicInt = new AtomicInt(1);

    arktest.assertEQ(atomicInt.compareAndSwap(1, 2), 1);
    arktest.assertEQ(atomicInt.load(), 2);
    arktest.assertEQ(atomicInt.compareAndSwap(1, 3), 2);
    arktest.assertEQ(atomicInt.load(), 2);       
}

function nonconcurrentFetchAdd() {
    let atomicInt = new AtomicInt(0);
    atomicInt.fetchAdd(5);
    arktest.assertEQ(atomicInt.load(), 5);
    atomicInt.fetchAdd(15);
    arktest.assertEQ(atomicInt.load(), 20);
}

function nonconcurrentFetchSub() {
    let atomicInt = new AtomicInt(32);
    atomicInt.fetchSub(8);
    arktest.assertEQ(atomicInt.load(), 24);
    atomicInt.fetchSub(10);
    arktest.assertEQ(atomicInt.load(), 14);
}

function nonconcurrentFetchAnd() {
    let atomicInt = new AtomicInt(15);
    atomicInt.fetchAnd(21);
    arktest.assertEQ(atomicInt.load(), 21 & 15);
    atomicInt.fetchAnd(2);
    arktest.assertEQ(atomicInt.load(), 21 & 15 & 2);
}

function nonconcurrentFetchOr() {
    let atomicInt = new AtomicInt(0);
    atomicInt.fetchOr(5);
    arktest.assertEQ(atomicInt.load(), 0 | 5);
    atomicInt.fetchOr(3);
    arktest.assertEQ(atomicInt.load(), 0 | 5 | 3);
}

function nonconcurrentFetchXor() {
    let atomicInt = new AtomicInt(2);
    atomicInt.fetchXor(5);
    arktest.assertEQ(atomicInt.load(), 2 ^ 5);
    atomicInt.fetchXor(5);
    arktest.assertEQ(atomicInt.load(), 2 ^ 5 ^ 5);
}

function concurrentLoadStoreTest() {
    let atomicInt = new AtomicInt(1);
    let count : int = 0;

    let worker = new EAWorker;     
    worker.start();
    worker.run<void>(() => {
        count = 1;
        atomicInt.store(2);
    });

    while(atomicInt.load() == 1) {
        // wait here
    }
    arktest.assertEQ(count, 1);
    worker.join();
}

// Checks correctness of visibility
function concurrentExchangeTest() {
    let atomicInt = new AtomicInt(1);
    let count : int = 0;

    let worker1 = new EAWorker;     
    worker1.start();
    worker1.run<void>(() => {
        count = 1;
        atomicInt.store(2);
    });

    let worker2 = new EAWorker;
    worker2.start();
    worker2.run<void>(() => {
        while(atomicInt.load() == 1) {
            // wait here
        }
        arktest.assertEQ(count, 1);
        count = 2;
        atomicInt.exchange(3);
    });

    while(atomicInt.load() != 3) {
        // wait here
    }
    arktest.assertEQ(count, 2);
    worker1.join();
    worker2.join();
}

function concurrentCompareAndSwapTest() {
    let atomicInt = new AtomicInt(1);

    let worker1 = new EAWorker;     
    let w1 = false;
    let worker2 = new EAWorker;
    let w2 = false;
    
    worker1.start();
    worker2.start();
    worker1.run<void>(() => {
        if (atomicInt.compareAndSwap(1, 2) == 1) {
            w1 = true;
        } else {
            arktest.assertEQ(atomicInt.load(), 3);
        }
    });
    worker2.run<void>(() => {
        if (atomicInt.compareAndSwap(1, 3) == 1) {
            w2 = true;
        } else {
            arktest.assertEQ(atomicInt.load(), 2);
        }
    });

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertTrue((w1 && !w2) || (!w1 && w2));
}

function concurrentFetchAdd() {
    let atomicInt = new AtomicInt(0);
    let worker1 = new EAWorker;     
    let worker2 = new EAWorker;
    let workerBody = () => {
        for (let i: int = 0; i < 10000; i++) {
            atomicInt.fetchAdd(1);
        }
    }
    worker1.start();
    worker2.start();

    worker1.run<void>(workerBody);
    worker2.run<void>(workerBody);

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertEQ(atomicInt.load(), 20000);
}

function concurrentFetchSub() {
    let atomicInt = new AtomicInt(20000);
    let worker1 = new EAWorker;     
    let worker2 = new EAWorker;
    let workerBody = () => {
        for (let i: int = 0; i < 10000; i++) {
            atomicInt.fetchSub(1);
        }
    }
    worker1.start();
    worker2.start();

    worker1.run<void>(workerBody);
    worker2.run<void>(workerBody);

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertEQ(atomicInt.load(), 0);
}

function concurrentFetchAnd() {
    let atomicInt = new AtomicInt(3);
    let worker1 = new EAWorker;     
    let worker2 = new EAWorker;
    let workerBody = (val: int) => {
        atomicInt.fetchAnd(val);
    }
    worker1.start();
    worker2.start();

    worker1.run<void>(workerBody, 1);
    worker2.run<void>(workerBody, 2);

    worker1.join().Await();
    worker2.join().Await();

    arktest.assertEQ(atomicInt.load(), 3 & 2 & 1);
}

function concurrentFetchOr() {
    let atomicInt = new AtomicInt(0);
    let worker1 = new EAWorker;     
    let worker2 = new EAWorker;
    let workerBody = (val: int) => {
        atomicInt.fetchXor(val);
    }
    worker1.start();
    worker2.start();

    worker1.run<void>(workerBody, 1);
    worker2.run<void>(workerBody, 2);

    worker1.join().Await();
    worker2.join().Await();

    arktest.assertEQ(atomicInt.load(), 0 | 1 | 2);
}

function concurrentFetchXor() {
    let atomicInt = new AtomicInt(1);
    let worker1 = new EAWorker;     
    let worker2 = new EAWorker;
    let workerBody = (val: int) => {
        atomicInt.fetchXor(val);
    }
    worker1.start();
    worker2.start();

    worker1.run<void>(workerBody, 1);
    worker2.run<void>(workerBody, 2);

    worker1.join().Await();
    worker2.join().Await();

    arktest.assertEQ(atomicInt.load(), 1 ^ 1 ^ 2);
}

function main() {
    const suite = new arktest.ArkTestsuite('AtomicsIntTest');
    suite.addTest('nonconcurrentLoadStoreTest', nonconcurrentLoadStoreTest);
    suite.addTest('nonconcurrentExchangeTest', nonconcurrentExchangeTest);
    suite.addTest('nonconcurrentCompareAndSwap', nonconcurrentCompareAndSwapTest);
    suite.addTest('nonconcurrentFetchAdd', nonconcurrentFetchAdd);
    suite.addTest('nonconcurrentFetchSub', nonconcurrentFetchSub);
    suite.addTest('nonconcurrentFetchAnd', nonconcurrentFetchAnd);
    suite.addTest('nonconcurrentFetchOr', nonconcurrentFetchOr);
    suite.addTest('nonconcurrentFetchXor', nonconcurrentFetchXor);
    suite.addTest('concurrentLoadStoreTest', concurrentLoadStoreTest);
    suite.addTest('concurrentExchangeTest', concurrentExchangeTest);
    suite.addTest('concurrentCompareAndSwap', concurrentCompareAndSwapTest);
    suite.addTest('concurrentFetchAdd', concurrentFetchAdd);
    suite.addTest('concurrentFetchSub', concurrentFetchSub);
    suite.addTest('concurrentFetchAnd', concurrentFetchAnd);
    suite.addTest('concurrentFetchOr', concurrentFetchOr);
    suite.addTest('concurrentFetchXor', concurrentFetchXor);
    return suite.run()
}
