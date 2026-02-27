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

import { AtomicReference } from "std/concurrency"

class Container {
    public value: int = 0;
}

function nonconcurrentLoadStoreTest() {
    let cont1 = new Container;
    cont1.value = 1;
    let atomicRef = new AtomicReference<Container>(cont1);
    arktest.assertEQ(atomicRef.load(), cont1);
    arktest.assertEQ(atomicRef.load().value, 1);

    let cont2 = new Container;
    cont2.value = 2;
    atomicRef.store(cont2);
    arktest.assertEQ(atomicRef.load(), cont2);
    arktest.assertEQ(atomicRef.load().value, 2)
}


function nonconcurrentExchangeTest() {
    let cont1 = new Container;
    let atomicRef = new AtomicReference<Container>(cont1);

    let cont2 = new Container;
    let prevCont = atomicRef.exchange(cont2);
    arktest.assertEQ(prevCont, cont1);
    arktest.assertEQ(atomicRef.load(), cont2);
}

function nonconcurrentCompareAndSwapTest() {
    let cont1 = new Container;
    let atomicRef = new AtomicReference<Container>(cont1);

    let cont2 = new Container;
    let cont3 = new Container;
    arktest.assertEQ(atomicRef.compareAndSwap(cont1, cont2), cont1);
    arktest.assertEQ(atomicRef.load(), cont2);
    arktest.assertEQ(atomicRef.compareAndSwap(cont1, cont3), cont2);
    arktest.assertEQ(atomicRef.load(), cont2);       
}

function concurrentLoadStoreTest() {
    let start : Container = new Container();
    let atomicRef = new AtomicReference<Container>(start);
    let count : int = 0;

    let worker = new EAWorker;     
    worker.start();
    worker.run<void>(() => {
        count = 1;
        atomicRef.store(new Container());
    });

    while(atomicRef.load() == start) {
        // wait here
    }
    arktest.assertEQ(count, 1);
    worker.join();
}

// Checks correctness of visibility
function concurrentExchangeTest() {
    let start : Container = new Container();
    let toStore : Container = new Container();
    let atomicRef = new AtomicReference<Container>(start);
    let count : int = 0;

    let worker1 = new EAWorker;     
    worker1.start();
    worker1.run<void>(() => {
        count = 1;
        atomicRef.store(new Container());
    });

    let worker2 = new EAWorker;
    worker2.start();
    worker2.run<void>(() => {
        while(atomicRef.load() == start) {
            // wait here
        }
        arktest.assertEQ(count, 1);
        count = 2;
        atomicRef.exchange(toStore);
    });

    while(atomicRef.load() != toStore) {
        // wait here
    }
    arktest.assertEQ(count, 2);
    worker1.join();
    worker2.join();
}

function concurrentCompareAndSwapTest() {
    let start = new Container();
    let atomicRef = new AtomicReference<Container>(start);

    let worker1 = new EAWorker;     
    let w1 = false;
    let worker2 = new EAWorker;
    let w2 = false;
    
    worker1.start();
    worker2.start();
    worker1.run<void>(() => {
        if (atomicRef.compareAndSwap(start, new Container()) == start) {
            w1 = true;
        }
    });
    worker2.run<void>(() => {
        if (atomicRef.compareAndSwap(start, new Container()) == start) {
            w2 = true;
        }
    });

    worker1.join().Await();
    worker2.join().Await();
    
    arktest.assertTrue((w1 && !w2) || (!w1 && w2));
}

function nonconcurrentNullStorage() {
    let atomicRef = new AtomicReference<Container | null>(null);
    let obj = new Container();

    arktest.assertEQ(atomicRef.load(), null);
    atomicRef.store(obj);
    arktest.assertEQ(atomicRef.load(), obj);
    arktest.assertEQ(atomicRef.exchange(null), obj);
    arktest.assertEQ(atomicRef.load(), null);
}

function main() {
    const suite = new arktest.ArkTestsuite('AtomicsRefTest');
    suite.addTest('nonconcurrentLoadStoreTest', nonconcurrentLoadStoreTest);
    suite.addTest('nonconcurrentExchangeTest', nonconcurrentExchangeTest);
    suite.addTest('nonconcurrentCompareAndSwap', nonconcurrentCompareAndSwapTest);
    suite.addTest('nonconcurrentNullStorage', nonconcurrentNullStorage);
    suite.addTest('concurrentLoadStoreTest', concurrentLoadStoreTest);
    suite.addTest('concurrentExchangeTest', concurrentExchangeTest);
    suite.addTest('concurrentCompareAndSwap', concurrentCompareAndSwapTest);
    return suite.run()
}
