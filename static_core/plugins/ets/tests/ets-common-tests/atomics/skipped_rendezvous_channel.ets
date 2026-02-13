/*
* Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

import { launch } from 'std/concurrency'

let s = new ArrayBuffer(4);
let arr = new Int32Array(s, 0, 1);

async function testMain() {
    let consumerWorker = new EAWorker();
    let producerWorker = new EAWorker();
    consumerWorker.start();
    producerWorker.start();
    let consumerJob = consumerWorker.run<Int>(consumer)
    heavyComputation()
    let producerJob = producerWorker.run<Int>(producer)

    producerJob.Await();
    consumerJob.Await();
    consumerWorker.join();
    producerWorker.join();
}

function producer(): Int {
    for (let i = 1; i < 10; i++) {
        send(i)
    }
    return 1
}

function consumer(): Int {
    for (let i = 1; i < 10; i++) {
        let x = read()
        arktest.assertEQ(x, i)
    }
    return 1
}

function send(x: int) {
    arktest.assertNE(x, 0)
    try {
        while (true) {
            let oldValue = Atomics.compareExchange(arr, 0, 0, x)
            if (oldValue == 0) {
                Atomics.notify(arr, 0)
                return
            }
            Atomics.wait(arr, 0, oldValue)
        }
    } catch (e) {
        arktest.assertTrue(false)
    }
}

function read(): int {
    try {
        while (true) {
            let value = Atomics.exchange(arr, 0, 0)
            if (value != 0) {
                Atomics.notify(arr, 0)
                return value.toInt();
            }
            Atomics.wait(arr, 0, 0)
        }
    } catch (e) {
        arktest.assertTrue(false)
    }
    return -1
}


function heavyComputation(): Int {
    let i = 1
    for (let j = 0; j < 10000; j++) {
        i += j
    }
    return i
}

function main() {
    const suite = new arktest.ArkTestsuite('Function Atomics skipped_rendezvous_channel tests')
    suite.addAsyncTest('Function testMain', testMain)
    return suite.run()
}
