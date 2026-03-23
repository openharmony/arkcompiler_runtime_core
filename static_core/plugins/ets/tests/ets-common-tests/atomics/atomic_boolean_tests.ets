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

import { AtomicBoolean } from 'std/concurrency'

function nonconcurrentLoadStoreTest() {
    let atomicBoolean = new AtomicBoolean(false);
    arktest.assertEQ(atomicBoolean.load(), false);

    atomicBoolean.store(true);
    arktest.assertEQ(atomicBoolean.load(), true)
}


function nonconcurrentExchangeTest() {
    let atomicBoolean = new AtomicBoolean(false);

    let prev = atomicBoolean.exchange(true);
    arktest.assertEQ(prev, false);
    arktest.assertEQ(atomicBoolean.load(), true);
}

function nonconcurrentCompareAndSwapTest() {
    let atomicBoolean = new AtomicBoolean(false);

    arktest.assertFalse(atomicBoolean.compareAndSwap(false, true));
    arktest.assertEQ(atomicBoolean.load(), true);
}

function concurrentLoadStoreTest() {
    let atomicBoolean = new AtomicBoolean(false);
    let count : int = 0;

    let worker = new EAWorker;     
    worker.start();
    worker.run<void>(() => {
        count = 1;
        atomicBoolean.store(true);
    });

    while(!atomicBoolean.load()) {
        // wait here
    }
    arktest.assertEQ(count, 1);
    worker.join();
}

// Checks correctness of visibility
function concurrentExchangeTest() {
    let atomicBoolean = new AtomicBoolean(false);
    let count : int = 0;

    let worker1 = new EAWorker;     
    worker1.start();
    worker1.run<void>(() => {
        count = 1;
        atomicBoolean.exchange(true);
    });

    while(!atomicBoolean.load()) {
        // wait here
    }
    arktest.assertEQ(count, 1);
    worker1.join();
}

function concurrentCompareAndSwapTest() {
    let atomicBoolean = new AtomicBoolean(false);

    let worker1 = new EAWorker;   
    let w1 = false;
    let worker2 = new EAWorker;
    let w2 = false;
    
    worker1.start();
    worker2.start();
    worker1.run<void>(() => {
        if (!atomicBoolean.compareAndSwap(false, true)) {
            w1 = true;
        } 
    });
    worker2.run<void>(() => {
        if (!atomicBoolean.compareAndSwap(false, true)) {
            w2 = true;
        }
    });

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertTrue((w1 && !w2) || (!w1 && w2));
}

function main() {
    const suite = new arktest.ArkTestsuite('AtomicsBooleanTest');
    suite.addTest('nonconcurrentLoadStoreTest', nonconcurrentLoadStoreTest);
    suite.addTest('nonconcurrentExchangeTest', nonconcurrentExchangeTest);
    suite.addTest('nonconcurrentCompareAndSwap', nonconcurrentCompareAndSwapTest);
    suite.addTest('concurrentLoadStoreTest', concurrentLoadStoreTest);
    suite.addTest('concurrentExchangeTest', concurrentExchangeTest);
    suite.addTest('concurrentCompareAndSwap', concurrentCompareAndSwapTest);
    return suite.run()
}

