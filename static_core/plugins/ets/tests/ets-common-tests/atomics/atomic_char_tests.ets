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

import { AtomicChar } from "std/concurrency"

function nonconcurrentLoadStoreTest() {
    let atomicChar = new AtomicChar(c'a');
    arktest.assertEQ(atomicChar.load(), c'a');

    atomicChar.store(c'b');
    arktest.assertEQ(atomicChar.load(), c'b')
}


function nonconcurrentExchangeTest() {
    let atomicChar = new AtomicChar(c'a');

    let prev = atomicChar.exchange(c'b');
    arktest.assertEQ(prev, c'a');
    arktest.assertEQ(atomicChar.load(), c'b');
}

function nonconcurrentCompareAndSwapTest() {
    let atomicChar = new AtomicChar(c'a');

    arktest.assertEQ(atomicChar.compareAndSwap(c'a', c'b'), c'a');
    arktest.assertEQ(atomicChar.load(), c'b');
    arktest.assertEQ(atomicChar.compareAndSwap(c'a', c'c'), c'b');
    arktest.assertEQ(atomicChar.load(), c'b');       
}

function concurrentLoadStoreTest() {
    let atomicChar = new AtomicChar(c'a');
    let count : int = 0;

    let worker = new EAWorker;     
    worker.start();
    worker.run<void>(() => {
        count = 1;
        atomicChar.store(c'b');
    });

    while(atomicChar.load() == c'a') {
        // wait here
    }
    arktest.assertEQ(count, 1);
    worker.join();
}

// Checks correctness of visibility
function concurrentExchangeTest() {
    let atomicChar = new AtomicChar(c'a');
    let count : int = 0;

    let worker1 = new EAWorker;     
    worker1.start();
    worker1.run<void>(() => {
        count = 1;
        atomicChar.store(c'b');
    });

    let worker2 = new EAWorker;
    worker2.start();
    worker2.run<void>(() => {
        while(atomicChar.load() == c'a') {
            // wait here
        }
        arktest.assertEQ(count, 1);
        count = 2;
        atomicChar.exchange(c'c');
    });

    while(atomicChar.load() != c'c') {
        // wait here
    }
    arktest.assertEQ(count, 2);
    worker1.join();
    worker2.join();
}

function concurrentCompareAndSwapTest() {
    let atomicChar = new AtomicChar(c'a');

    let worker1 = new EAWorker;     
    let w1 = false;
    let worker2 = new EAWorker;
    let w2 = false;
    
    worker1.start();
    worker2.start();
    worker1.run<void>(() => {
        if (atomicChar.compareAndSwap(c'a', c'b') == c'a') {
            w1 = true;
        } else {
            arktest.assertEQ(atomicChar.load(), c'c');
        }
    });
    worker2.run<void>(() => {
        if (atomicChar.compareAndSwap(c'a', c'c') == c'a') {
            w2 = true;
        } else {
            arktest.assertEQ(atomicChar.load(), c'b');
        }
    });

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertTrue((w1 && !w2) || (!w1 && w2));
}

function main() {
    const suite = new arktest.ArkTestsuite('AtomicsCharTest');
    suite.addTest('nonconcurrentLoadStoreTest', nonconcurrentLoadStoreTest);
    suite.addTest('nonconcurrentExchangeTest', nonconcurrentExchangeTest);
    suite.addTest('nonconcurrentCompareAndSwap', nonconcurrentCompareAndSwapTest);
    suite.addTest('concurrentLoadStoreTest', concurrentLoadStoreTest);
    suite.addTest('concurrentExchangeTest', concurrentExchangeTest);
    suite.addTest('concurrentCompareAndSwap', concurrentCompareAndSwapTest);
    return suite.run()
}
