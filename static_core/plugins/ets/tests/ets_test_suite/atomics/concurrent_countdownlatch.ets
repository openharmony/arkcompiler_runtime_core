/*
* Copyright (c) 2023 Huawei Device Co., Ltd.
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

let N = 9

let buf: SharedArrayBuffer;
let arr: Int32Array;
let bytearr: Int8Array;

function main() {
    let c = new Console() // workaround, issue: #12996

    buf = new SharedArrayBuffer(4)
    arr = new Int32Array(buf, 0, 1)
    bytearr = new Int8Array(buf, 0, 4)

    for (let i = 0; i < N; i++) {
        launch task()
    }

    let cur: byte = Atomics.load(bytearr, 0)
    while (cur != N) {
        c.println("waiting, finished tasks: " + cur)
        Atomics.wait(arr, 0, cur)
        cur = Atomics.load(bytearr, 0)
    }
    c.println("all tasks have finished")
}

function task(): Int {
    let c = new Console() // workaround, issue: #12996

    heavyComputation()
    c.println("Task finished. Notifying")
    Atomics.add(bytearr, 0, 1 as byte)
    Atomics.notify(arr, 0)
    
    return 1
}

function heavyComputation(): Int {
    let i = 1
    for (let j = 0; j < 10000; j++) {
        i += j
    }
    return i
}
