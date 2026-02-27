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
r*/

import { AtomicShort } from "std/concurrency"

function nonconcurrentLoadStoreTest() {
    let atomicShort = new AtomicShort(1);
    arktest.assertEQ(atomicShort.load(), 1);

    atomicShort.store(2);
    arktest.assertEQ(atomicShort.load(), 2)
}


function nonconcurrentExchangeTest() {
    let atomicShort = new AtomicShort(1);

    let prev = atomicShort.exchange(2);
    arktest.assertEQ(prev, 1);
    arktest.assertEQ(atomicShort.load(), 2);
}

function nonconcurrentCompareAndSwapTest() {
    let atomicShort = new AtomicShort(1);

    arktest.assertEQ(atomicShort.compareAndSwap(1, 2), 1);
    arktest.assertEQ(atomicShort.load(), 2);
    arktest.assertEQ(atomicShort.compareAndSwap(1, 3), 2);
    arktest.assertEQ(atomicShort.load(), 2);       
}

function nonconcurrentFetchAdd() {
    let atomicShort = new AtomicShort(0);
    atomicShort.fetchAdd(5);
    arktest.assertEQ(atomicShort.load(), 5);
    atomicShort.fetchAdd(15);
    arktest.assertEQ(atomicShort.load(), 20);
}

function nonconcurrentFetchSub() {
    let atomicShort = new AtomicShort(32);
    atomicShort.fetchSub(8);
    arktest.assertEQ(atomicShort.load(), 24);
    atomicShort.fetchSub(10);
    arktest.assertEQ(atomicShort.load(), 14);
}

function nonconcurrentFetchAnd() {
    let atomicShort = new AtomicShort(15);
    atomicShort.fetchAnd(21);
    arktest.assertEQ(atomicShort.load(), 21 & 15);
    atomicShort.fetchAnd(2);
    arktest.assertEQ(atomicShort.load(), 21 & 15 & 2);
}

function nonconcurrentFetchOr() {
    let atomicShort = new AtomicShort(0);
    atomicShort.fetchOr(5);
    arktest.assertEQ(atomicShort.load(), 0 | 5);
    atomicShort.fetchOr(3);
    arktest.assertEQ(atomicShort.load(), 0 | 5 | 3);
}

function nonconcurrentFetchXor() {
    let atomicShort = new AtomicShort(2);
    atomicShort.fetchXor(5);
    arktest.assertEQ(atomicShort.load(), 2 ^ 5);
    atomicShort.fetchXor(5);
    arktest.assertEQ(atomicShort.load(), 2 ^ 5 ^ 5);
}

function concurrentLoadStoreTest() {
    let atomicShort = new AtomicShort(1);
    let count : short = 0;

    let worker = new EAWorker;     
    worker.start();
    worker.run<void>(() => {
        count = 1;
        atomicShort.store(2);
    });

    while(atomicShort.load() == 1) {
        // wait here
    }
    arktest.assertEQ(count, 1);
    worker.join();
}

// Checks correctness of visibility
function concurrentExchangeTest() {
    let atomicShort = new AtomicShort(1);
    let count : short = 0;

    let worker1 = new EAWorker;     
    worker1.start();
    worker1.run<void>(() => {
        count = 1;
        atomicShort.store(2);
    });

    let worker2 = new EAWorker;
    worker2.start();
    worker2.run<void>(() => {
        while(atomicShort.load() == 1) {
            // wait here
        }
        arktest.assertEQ(count, 1);
        count = 2;
        atomicShort.exchange(3);
    });

    while(atomicShort.load() != 3) {
        // wait here
    }
    arktest.assertEQ(count, 2);
    worker1.join();
    worker2.join();
}

function concurrentCompareAndSwapTest() {
    let atomicShort = new AtomicShort(1);

    let worker1 = new EAWorker;     
    let w1 = false;
    let worker2 = new EAWorker;
    let w2 = false;
    
    worker1.start();
    worker2.start();
    worker1.run<void>(() => {
        if (atomicShort.compareAndSwap(1, 2) == 1) {
            w1 = true;
        } else {
            arktest.assertEQ(atomicShort.load(), 3);
        }
    });
    worker2.run<void>(() => {
        if (atomicShort.compareAndSwap(1, 3) == 1) {
            w2 = true;
        } else {
            arktest.assertEQ(atomicShort.load(), 2);
        }
    });

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertTrue((w1 && !w2) || (!w1 && w2));
}

function concurrentFetchAdd() {
    let atomicShort = new AtomicShort(0);
    let worker1 = new EAWorker;     
    let worker2 = new EAWorker;
    let workerBody = () => {
        for (let i = 0; i < 100; i++) {
            atomicShort.fetchAdd(1);
        }
    }
    worker1.start();
    worker2.start();

    worker1.run<void>(workerBody);
    worker2.run<void>(workerBody);

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertEQ(atomicShort.load(), 200);
}

function concurrentFetchSub() {
    let atomicShort = new AtomicShort(200);
    let worker1 = new EAWorker;     
    let worker2 = new EAWorker;
    let workerBody = () => {
        for (let i = 0; i < 100; i++) {
            atomicShort.fetchSub(1);
        }
    }
    worker1.start();
    worker2.start();

    worker1.run<void>(workerBody);
    worker2.run<void>(workerBody);

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertEQ(atomicShort.load(), 0);
}

function concurrentFetchAnd() {
        let atomicShort = new AtomicShort(3);
        let worker1 = new EAWorker;     
        let worker2 = new EAWorker;
        worker1.start();
        worker2.start();

        worker2.run<void>(() => {
            atomicShort.fetchAnd(1);
        });
        worker2.run<void>(() => {
            atomicShort.fetchAnd(2);
        });

        worker1.join().Await();
        worker2.join().Await();

        arktest.assertEQ(atomicShort.load(), 0);
}

function concurrentFetchOr() {
        let atomicShort = new AtomicShort(0);
        let worker1 = new EAWorker;     
        let worker2 = new EAWorker;
        worker1.start();
        worker2.start();

        worker1.run<void>(() => {
            atomicShort.fetchOr(1);
        });
        worker1.run<void>(() => {
            atomicShort.fetchOr(2);
        });

        worker1.join().Await();
        worker2.join().Await();

        arktest.assertEQ(atomicShort.load(), 3);
}

function concurrentFetchXor() {
        let atomicShort = new AtomicShort(1);
        let worker1 = new EAWorker;     
        let worker2 = new EAWorker;
        worker1.start();
        worker2.start();

        worker1.run<void>(() => {
            atomicShort.fetchXor(1);
        });
        worker1.run<void>(() => {
            atomicShort.fetchXor(2);
        });

        worker1.join().Await();
        worker2.join().Await();

        arktest.assertEQ(atomicShort.load(), 2);
}

function main() {
    const suite = new arktest.ArkTestsuite('AtomicsShortTest');
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

