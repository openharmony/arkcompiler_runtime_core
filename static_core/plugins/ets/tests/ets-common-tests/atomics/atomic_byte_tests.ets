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

import { AtomicByte } from "std/concurrency"

function nonconcurrentLoadStoreTest() {
    let atomicByte = new AtomicByte(1);
    arktest.assertEQ(atomicByte.load(), 1);

    atomicByte.store(2);
    arktest.assertEQ(atomicByte.load(), 2)
}


function nonconcurrentExchangeTest() {
    let atomicByte = new AtomicByte(1);

    let prev = atomicByte.exchange(2);
    arktest.assertEQ(prev, 1);
    arktest.assertEQ(atomicByte.load(), 2);
}

function nonconcurrentCompareAndSwapTest() {
    let atomicByte = new AtomicByte(1);

    arktest.assertEQ(atomicByte.compareAndSwap(1, 2), 1);
    arktest.assertEQ(atomicByte.load(), 2);
    arktest.assertEQ(atomicByte.compareAndSwap(1, 3), 2);
    arktest.assertEQ(atomicByte.load(), 2);       
}

function nonconcurrentFetchAdd() {
    let atomicByte = new AtomicByte(0);
    atomicByte.fetchAdd(5);
    arktest.assertEQ(atomicByte.load(), 5);
    atomicByte.fetchAdd(15);
    arktest.assertEQ(atomicByte.load(), 20);
}

function nonconcurrentFetchSub() {
    let atomicByte = new AtomicByte(32);
    atomicByte.fetchSub(8);
    arktest.assertEQ(atomicByte.load(), 24);
    atomicByte.fetchSub(10);
    arktest.assertEQ(atomicByte.load(), 14);
}

function nonconcurrentFetchAnd() {
    let atomicByte = new AtomicByte(15);
    atomicByte.fetchAnd(21);
    arktest.assertEQ(atomicByte.load(), 21 & 15);
    atomicByte.fetchAnd(2);
    arktest.assertEQ(atomicByte.load(), 21 & 15 & 2);
}

function nonconcurrentFetchOr() {
    let atomicByte = new AtomicByte(0);
    atomicByte.fetchOr(5);
    arktest.assertEQ(atomicByte.load(), 0 | 5);
    atomicByte.fetchOr(3);
    arktest.assertEQ(atomicByte.load(), 0 | 5 | 3);
}

function nonconcurrentFetchXor() {
    let atomicByte = new AtomicByte(2);
    atomicByte.fetchXor(5);
    arktest.assertEQ(atomicByte.load(), 2 ^ 5);
    atomicByte.fetchXor(5);
    arktest.assertEQ(atomicByte.load(), 2 ^ 5 ^ 5);
}

function concurrentLoadStoreTest() {
    let atomicByte = new AtomicByte(1);
    let count : byte = 0;

    let worker = new EAWorker;     
    worker.start();
    worker.run<void>(() => {
        count = 1;
        atomicByte.store(2);
    });

    while(atomicByte.load() == 1) {
        // wait here
    }
    arktest.assertEQ(count, 1);
    worker.join();
}

// Checks correctness of visibility
function concurrentExchangeTest() {
    let atomicByte = new AtomicByte(1);
    let count : byte = 0;

    let worker1 = new EAWorker;     
    worker1.start();
    worker1.run<void>(() => {
        count = 1;
        atomicByte.store(2);
    });

    let worker2 = new EAWorker;
    worker2.start();
    worker2.run<void>(() => {
        while(atomicByte.load() == 1) {
            // wait here
        }
        arktest.assertEQ(count, 1);
        count = 2;
        atomicByte.exchange(3);
    });

    while(atomicByte.load() != 3) {
        // wait here
    }
    arktest.assertEQ(count, 2);
    worker1.join();
    worker2.join();
}

function concurrentCompareAndSwapTest() {
    let atomicByte = new AtomicByte(1);

    let worker1 = new EAWorker;     
    let w1 = false;
    let worker2 = new EAWorker;
    let w2 = false;
    
    worker1.start();
    worker2.start();
    worker1.run<void>(() => {
        if (atomicByte.compareAndSwap(1, 2) == 1) {
            w1 = true;
        } else {
            arktest.assertEQ(atomicByte.load(), 3);
        }
    });
    worker2.run<void>(() => {
        if (atomicByte.compareAndSwap(1, 3) == 1) {
            w2 = true;
        } else {
            arktest.assertEQ(atomicByte.load(), 2);
        }
    });

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertTrue((w1 && !w2) || (!w1 && w2));
}

function concurrentFetchAdd() {
    let atomicByte = new AtomicByte(1);
    let worker1 = new EAWorker;     
    let worker2 = new EAWorker;
    let workerBody = () => {
        for (let i: byte = 0; i < 3; i++) {
            atomicByte.fetchAdd(1);
        }
    }
    worker1.start();
    worker2.start();

    worker1.run<void>(workerBody);
    worker2.run<void>(workerBody);

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertEQ(atomicByte.load(), 7);
}

function concurrentFetchSub() {
    let atomicByte = new AtomicByte(7);
    let worker1 = new EAWorker;     
    let worker2 = new EAWorker;
    let workerBody = () => {
        for (let i: byte = 0; i < 3; i++) {
            atomicByte.fetchSub(1);
        }
    }
    worker1.start();
    worker2.start();

    worker1.run<void>(workerBody);
    worker2.run<void>(workerBody);

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertEQ(atomicByte.load(), 1);
}

function concurrentFetchAnd() {
        let atomicByte = new AtomicByte(3);
        let worker1 = new EAWorker;     
        let worker2 = new EAWorker;
        worker1.start();
        worker2.start();

        worker1.run<void>(() => {
            atomicByte.fetchXor(1);
        });
        worker2.run<void>(() => {
            atomicByte.fetchXor(2);
        });

        worker1.join().Await();
        worker2.join().Await();

        arktest.assertEQ(atomicByte.load(), 0);
}

function concurrentFetchOr() {
        let atomicByte = new AtomicByte(0);
        let worker1 = new EAWorker;     
        let worker2 = new EAWorker;
        worker1.start();
        worker2.start();

        worker1.run<void>(() => {
            atomicByte.fetchOr(1);
        });
        worker2.run<void>(() => {
            atomicByte.fetchOr(2);
        });

        worker1.join().Await();
        worker2.join().Await();

        arktest.assertEQ(atomicByte.load(), 3);
}

function concurrentFetchXor() {
        let atomicByte = new AtomicByte(1);
        let worker1 = new EAWorker;     
        let worker2 = new EAWorker;
    
        worker1.start();
        worker2.start();

        worker1.run<void>(() => {
            atomicByte.fetchXor(1);
        });
        worker2.run<void>(() => {
            atomicByte.fetchXor(2);
        });

        worker1.join().Await();
        worker2.join().Await();

        arktest.assertEQ(atomicByte.load(), 2);
}

function main() {
    const suite = new arktest.ArkTestsuite('AtomicsByteTest');
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

