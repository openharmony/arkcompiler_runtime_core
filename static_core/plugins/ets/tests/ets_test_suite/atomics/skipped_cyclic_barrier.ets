/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

let s = new SharedArrayBuffer(4);
let arr = new Int32Array(s, 0, 1);

let N = 5

function main() {
    let corArrayTmp = new Array<Promise<int>>();
    setUpBarrier(N)
    for (let i = 0; i < N; i++) {
        corArrayTmp.push(launch task());
    }
    for (let i = 0; i < N; i++) {
        await corArrayTmp[i];
    }
}

function task(): Int {
    let c = new Console() // workaround, issue: #12996 

    heavyComputation()
    c.println("task completed, waiting")
    wait()
    c.println("wait completed")
    return 1
}

function setUpBarrier(nTasks: int) {
    try {
        Atomics.store(arr, 0, nTasks)
    } catch (e) {
        assertTrue(false)
    }
}

function wait() {
    try {
        Atomics.sub(arr, 0, 1)
        Atomics.notify(arr, 0)

        // wait until is 0
        let cur = Atomics.load(arr, 0)
        while (cur != 0) {
            Atomics.wait(arr, 0, cur)
            cur = Atomics.load(arr, 0)
        }
    } catch (e) {
        assertTrue(false)
    }
}


function heavyComputation(): Int {
    let i = 1
    for (let j = 0; j < 10000; j++) {
        i += j
    }
    return i
}

