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

import { AtomicFloat } from "std/concurrency"

function nonconcurrentLoadStoreTest() {
    let atomicFloat = new AtomicFloat(1);
    arktest.assertEQ(atomicFloat.load(), 1);

    atomicFloat.store(2);
    arktest.assertEQ(atomicFloat.load(), 2)
}


function nonconcurrentExchangeTest() {
    let atomicFloat = new AtomicFloat(1);

    let prev = atomicFloat.exchange(2);
    arktest.assertEQ(prev, 1);
    arktest.assertEQ(atomicFloat.load(), 2);
}

function nonconcurrentCompareAndSwapTest() {
    let atomicFloat = new AtomicFloat(1);

    arktest.assertEQ(atomicFloat.compareAndSwap(1.0, 2.0), 1.0);
    arktest.assertEQ(atomicFloat.load(), 2);
    arktest.assertEQ(atomicFloat.compareAndSwap(1, 3), 2);
    arktest.assertEQ(atomicFloat.load(), 2);       
}

function nonconcurrentFetchAdd() {
    let atomicFloat = new AtomicFloat(0);
    atomicFloat.fetchAdd(5);
    arktest.assertEQ(atomicFloat.load(), 5);
    atomicFloat.fetchAdd(15);
    arktest.assertEQ(atomicFloat.load(), 20);
}

function nonconcurrentFetchSub() {
    let atomicFloat = new AtomicFloat(32);
    atomicFloat.fetchSub(8);
    arktest.assertEQ(atomicFloat.load(), 24);
    atomicFloat.fetchSub(10);
    arktest.assertEQ(atomicFloat.load(), 14);
}

function concurrentLoadStoreTest() {
    let atomicFloat = new AtomicFloat(1);
    let count : float = 0;

    let worker = new EAWorker;     
    worker.start();
    worker.run<void>(() => {
        count = 1;
        atomicFloat.store(2);
    });

    while(atomicFloat.load() == 1) {
        // wait here
    }
    arktest.assertEQ(count, 1);
    worker.join();
}

// Checks correctness of visibility
function concurrentExchangeTest() {
    let atomicFloat = new AtomicFloat(1);
    let count : float = 0;

    let worker1 = new EAWorker;     
    worker1.start();
    worker1.run<void>(() => {
        count = 1;
        atomicFloat.store(2);
    });

    let worker2 = new EAWorker;
    worker2.start();
    worker2.run<void>(() => {
        while(atomicFloat.load() == 1) {
            // wait here
        }
        arktest.assertEQ(count, 1);
        count = 2;
        atomicFloat.exchange(3);
    });

    while(atomicFloat.load() != 3) {
        // wait here
    }
    arktest.assertEQ(count, 2);
    worker1.join();
    worker2.join();
}

function concurrentCompareAndSwapTest() {
    let atomicFloat = new AtomicFloat(1);

    let worker1 = new EAWorker;     
    let w1 = false;
    let worker2 = new EAWorker;
    let w2 = false;
    
    worker1.start();
    worker2.start();
    worker1.run<void>(() => {
        if (atomicFloat.compareAndSwap(1, 2) == 1) {
            w1 = true;
        } else {
            arktest.assertEQ(atomicFloat.load(), 3);
        }
    });
    worker2.run<void>(() => {
        if (atomicFloat.compareAndSwap(1, 3) == 1) {
            w2 = true;
        } else {
            arktest.assertEQ(atomicFloat.load(), 2);
        }
    });

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertTrue((w1 && !w2) || (!w1 && w2));
}

function concurrentFetchAdd() {
    let atomicFloat = new AtomicFloat(0);
    let worker1 = new EAWorker;     
    let worker2 = new EAWorker;
    let workerBody = () => {
        for (let i: float = 0; i < 10000; i++) {
            atomicFloat.fetchAdd(1);
        }
    }
    worker1.start();
    worker2.start();

    worker1.run<void>(workerBody);
    worker2.run<void>(workerBody);

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertEQ(atomicFloat.load(), 20000);
}

function concurrentFetchSub() {
    let atomicFloat = new AtomicFloat(20000);
    let worker1 = new EAWorker;     
    let worker2 = new EAWorker;
    let workerBody = () => {
        for (let i: float = 0; i < 10000; i++) {
            atomicFloat.fetchSub(1);
        }
    }
    worker1.start();
    worker2.start();

    worker1.run<void>(workerBody);
    worker2.run<void>(workerBody);

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertEQ(atomicFloat.load(), 0);
}

function main() {
    const suite = new arktest.ArkTestsuite('AtomicsFloatTest');
    suite.addTest('nonconcurrentLoadStoreTest', nonconcurrentLoadStoreTest);
    suite.addTest('nonconcurrentExchangeTest', nonconcurrentExchangeTest);
    suite.addTest('nonconcurrentCompareAndSwap', nonconcurrentCompareAndSwapTest);
    suite.addTest('nonconcurrentFetchAdd', nonconcurrentFetchAdd);
    suite.addTest('nonconcurrentFetchSub', nonconcurrentFetchSub);
    suite.addTest('concurrentLoadStoreTest', concurrentLoadStoreTest);
    suite.addTest('concurrentExchangeTest', concurrentExchangeTest);
    suite.addTest('concurrentCompareAndSwap', concurrentCompareAndSwapTest);
    suite.addTest('concurrentFetchAdd', concurrentFetchAdd);
    suite.addTest('concurrentFetchSub', concurrentFetchSub);
    return suite.run()
}

