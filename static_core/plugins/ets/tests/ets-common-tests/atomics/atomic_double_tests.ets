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

import { AtomicDouble } from "std/concurrency"

function nonconcurrentLoadStoreTest() {
    let atomicDouble = new AtomicDouble(1);
    arktest.assertEQ(atomicDouble.load(), 1);

    atomicDouble.store(2);
    arktest.assertEQ(atomicDouble.load(), 2)
}


function nonconcurrentExchangeTest() {
    let atomicDouble = new AtomicDouble(1);

    let prev = atomicDouble.exchange(2);
    arktest.assertEQ(prev, 1);
    arktest.assertEQ(atomicDouble.load(), 2);
}

function nonconcurrentCompareAndSwapTest() {
    let atomicDouble = new AtomicDouble(1);

    arktest.assertEQ(atomicDouble.compareAndSwap(1.0, 2.0), 1.0);
    arktest.assertEQ(atomicDouble.load(), 2);
    arktest.assertEQ(atomicDouble.compareAndSwap(1, 3), 2);
    arktest.assertEQ(atomicDouble.load(), 2);       
}

function nonconcurrentFetchAdd() {
    let atomicDouble = new AtomicDouble(0);
    atomicDouble.fetchAdd(5);
    arktest.assertEQ(atomicDouble.load(), 5);
    atomicDouble.fetchAdd(15);
    arktest.assertEQ(atomicDouble.load(), 20);
}

function nonconcurrentFetchSub() {
    let atomicDouble = new AtomicDouble(32);
    atomicDouble.fetchSub(8);
    arktest.assertEQ(atomicDouble.load(), 24);
    atomicDouble.fetchSub(10);
    arktest.assertEQ(atomicDouble.load(), 14);
}

function concurrentLoadStoreTest() {
    let atomicDouble = new AtomicDouble(1);
    let count : double = 0;

    let worker = new EAWorker;     
    worker.start();
    worker.run<void>(() => {
        count = 1;
        atomicDouble.store(2);
    });

    while(atomicDouble.load() == 1) {
        // wait here
    }
    arktest.assertEQ(count, 1);
    worker.join();
}

// Checks correctness of visibility
function concurrentExchangeTest() {
    let atomicDouble = new AtomicDouble(1);
    let count : double = 0;

    let worker1 = new EAWorker;     
    worker1.start();
    worker1.run<void>(() => {
        count = 1;
        atomicDouble.store(2);
    });

    let worker2 = new EAWorker;
    worker2.start();
    worker2.run<void>(() => {
        while(atomicDouble.load() == 1) {
            // wait here
        }
        arktest.assertEQ(count, 1);
        count = 2;
        atomicDouble.exchange(3);
    });

    while(atomicDouble.load() != 3) {
        // wait here
    }
    arktest.assertEQ(count, 2);
    worker1.join();
    worker2.join();
}

function concurrentCompareAndSwapTest() {
    let atomicDouble = new AtomicDouble(1);

    let worker1 = new EAWorker;     
    let w1 = false;
    let worker2 = new EAWorker;
    let w2 = false;
    
    worker1.start();
    worker2.start();
    worker1.run<void>(() => {
        if (atomicDouble.compareAndSwap(1, 2) == 1) {
            w1 = true;
        } else {
            arktest.assertEQ(atomicDouble.load(), 3);
        }
    });
    worker2.run<void>(() => {
        if (atomicDouble.compareAndSwap(1, 3) == 1) {
            w2 = true;
        } else {
            arktest.assertEQ(atomicDouble.load(), 2);
        }
    });

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertTrue((w1 && !w2) || (!w1 && w2));
}

function concurrentFetchAdd() {
    let atomicDouble = new AtomicDouble(0);
    let worker1 = new EAWorker;     
    let worker2 = new EAWorker;
    let workerBody = () => {
        for (let i: double = 0; i < 10000; i++) {
            atomicDouble.fetchAdd(1);
        }
    }
    worker1.start();
    worker2.start();

    worker1.run<void>(workerBody);
    worker2.run<void>(workerBody);

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertEQ(atomicDouble.load(), 20000);
}

function concurrentFetchSub() {
    let atomicDouble = new AtomicDouble(20000);
    let worker1 = new EAWorker;     
    let worker2 = new EAWorker;
    let workerBody = () => {
        for (let i: double = 0; i < 10000; i++) {
            atomicDouble.fetchSub(1);
        }
    }
    worker1.start();
    worker2.start();

    worker1.run<void>(workerBody);
    worker2.run<void>(workerBody);

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertEQ(atomicDouble.load(), 0);
}

function main() {
    const suite = new arktest.ArkTestsuite('AtomicsDoubleTest');
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

