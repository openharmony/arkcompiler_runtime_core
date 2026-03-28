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

import { AtomicLong } from 'std/concurrency'

function nonconcurrentLoadStoreTest() {
    let atomicLong = new AtomicLong(1);
    arktest.assertEQ(atomicLong.load(), 1);

    atomicLong.store(2);
    arktest.assertEQ(atomicLong.load(), 2)
}

function nonconcurrentExchangeTest() {
    let atomicLong = new AtomicLong(1);

    let prev = atomicLong.exchange(2);
    arktest.assertEQ(prev, 1);
    arktest.assertEQ(atomicLong.load(), 2);
}

function nonconcurrentCompareAndSwapTest() {
    let atomicLong = new AtomicLong(1);

    arktest.assertEQ(atomicLong.compareAndSwap(1, 2), 1);
    arktest.assertEQ(atomicLong.load(), 2);
    arktest.assertEQ(atomicLong.compareAndSwap(1, 3), 2);
    arktest.assertEQ(atomicLong.load(), 2);       
}

function nonconcurrentFetchAdd() {
    let atomicLong = new AtomicLong(0);
    atomicLong.fetchAdd(5);
    arktest.assertEQ(atomicLong.load(), 5);
    atomicLong.fetchAdd(15);
    arktest.assertEQ(atomicLong.load(), 20);
}

function nonconcurrentFetchSub() {
    let atomicLong = new AtomicLong(32);
    atomicLong.fetchSub(8);
    arktest.assertEQ(atomicLong.load(), 24);
    atomicLong.fetchSub(10);
    arktest.assertEQ(atomicLong.load(), 14);
}

function nonconcurrentFetchAnd() {
    let atomicLong = new AtomicLong(15);
    atomicLong.fetchAnd(21);
    arktest.assertEQ(atomicLong.load(), 21 & 15);
    atomicLong.fetchAnd(2);
    arktest.assertEQ(atomicLong.load(), 21 & 15 & 2);
}

function nonconcurrentFetchOr() {
    let atomicLong = new AtomicLong(0);
    atomicLong.fetchOr(5);
    arktest.assertEQ(atomicLong.load(), 0 | 5);
    atomicLong.fetchOr(3);
    arktest.assertEQ(atomicLong.load(), 0 | 5 | 3);
}

function nonconcurrentFetchXor() {
    let atomicLong = new AtomicLong(2);
    atomicLong.fetchXor(5);
    arktest.assertEQ(atomicLong.load(), 2 ^ 5);
    atomicLong.fetchXor(5);
    arktest.assertEQ(atomicLong.load(), 2 ^ 5 ^ 5);
}

function concurrentLoadStoreTest() {
    let atomicLong = new AtomicLong(1);
    let count : long = 0;

    let worker = new EAWorker;     
    worker.start();
    worker.run<void>(() => {
        count = 1;
        atomicLong.store(2);
    });

    while(atomicLong.load() == 1) {
        // wait here
    }
    arktest.assertEQ(count, 1);
    worker.join();
}

// Checks correctness of visibility
function concurrentExchangeTest() {
    let atomicLong = new AtomicLong(1);
    let count : long = 0;

    let worker1 = new EAWorker;     
    worker1.start();
    worker1.run<void>(() => {
        count = 1;
        atomicLong.store(2);
    });

    let worker2 = new EAWorker;
    worker2.start();
    worker2.run<void>(() => {
        while(atomicLong.load() == 1) {
            // wait here
        }
        arktest.assertEQ(count, 1);
        count = 2;
        atomicLong.exchange(3);
    });

    while(atomicLong.load() != 3) {
        // wait here
    }
    arktest.assertEQ(count, 2);
    worker1.join();
    worker2.join();
}

function concurrentCompareAndSwapTest() {
    let atomicLong = new AtomicLong(1);

    let worker1 = new EAWorker;     
    let w1 = false;
    let worker2 = new EAWorker;
    let w2 = false;
    
    worker1.start();
    worker2.start();
    worker1.run<void>(() => {
        if (atomicLong.compareAndSwap(1, 2) == 1) {
            w1 = true;
        } else {
            arktest.assertEQ(atomicLong.load(), 3);
        }
    });
    worker2.run<void>(() => {
        if (atomicLong.compareAndSwap(1, 3) == 1) {
            w2 = true;
        } else {
            arktest.assertEQ(atomicLong.load(), 2);
        }
    });

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertTrue((w1 && !w2) || (!w1 && w2));
}

function concurrentFetchAdd() {
    let atomicLong = new AtomicLong(0);
    let worker1 = new EAWorker;     
    let worker2 = new EAWorker;
    let workerBody = () => {
        for (let i: long = 0; i < 10000; i++) {
            atomicLong.fetchAdd(1);
        }
    }
    worker1.start();
    worker2.start();

    worker1.run<void>(workerBody);
    worker2.run<void>(workerBody);

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertEQ(atomicLong.load(), 20000);
}

function concurrentFetchSub() {
    let atomicLong = new AtomicLong(20000);
    let worker1 = new EAWorker;     
    let worker2 = new EAWorker;
    let workerBody = () => {
        for (let i: long = 0; i < 10000; i++) {
            atomicLong.fetchSub(1);
        }
    }
    worker1.start();
    worker2.start();

    worker1.run<void>(workerBody);
    worker2.run<void>(workerBody);

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertEQ(atomicLong.load(), 0);
}

function concurrentFetchAnd() {
        let atomicLong = new AtomicLong(3);
        let worker1 = new EAWorker;     
        let worker2 = new EAWorker;
        worker1.start();
        worker2.start();

        worker1.run<void>(() => {
            atomicLong.fetchAnd(1);
        });
        worker2.run<void>(() => {
            atomicLong.fetchAnd(2);
        });

        worker1.join().Await();
        worker2.join().Await();

        arktest.assertEQ(atomicLong.load(), 0);
}

function concurrentFetchOr() {
        let atomicLong = new AtomicLong(0);
        let worker1 = new EAWorker;     
        let worker2 = new EAWorker;
        
        worker1.start();
        worker2.start();

        worker1.run<void>(() => {
            atomicLong.fetchOr(1);
        });
        worker2.run<void>(() => {
            atomicLong.fetchOr(2);
        });

        worker1.join().Await();
        worker2.join().Await();

        arktest.assertEQ(atomicLong.load(), 3);
}

function concurrentFetchXor() {
        let atomicLong = new AtomicLong(1);
        let worker1 = new EAWorker;     
        let worker2 = new EAWorker;
        let workerBody = (val: long) => {
            atomicLong.fetchXor(val);
        }
        worker1.start();
        worker2.start();

        worker1.run<void>(() => {
            atomicLong.fetchXor(1);
        });
        worker2.run<void>(() => {
            atomicLong.fetchXor(2);
        });

        worker1.join().Await();
        worker2.join().Await();

        arktest.assertEQ(atomicLong.load(), 2);
}

function main() {
    const suite = new arktest.ArkTestsuite('AtomicsLongTest');
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

