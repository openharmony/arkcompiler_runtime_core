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

import {StdDebug} from "std/debug"

type L = StdDebug.Logger

let N = 9

let buf_1: SharedArrayBuffer;
let buf_2: SharedArrayBuffer;
let arr_1: Int32Array;
let arr_2: BigInt64Array;
let bytearr_i8: Int8Array;
let bytearr_i64: BigInt64Array;

function main() {
    buf_1 = new SharedArrayBuffer(4)
    buf_2 = new SharedArrayBuffer(8)
    arr_1 = new Int32Array(buf_1, 0, 1)
    arr_2 = new BigInt64Array(buf_2, 0, 1)
    bytearr_i8 = new Int8Array(buf_1, 0, 4)
    bytearr_i64 = new BigInt64Array(buf_2, 0, 1)

    for (let i = 0; i < N; i++) {
        launch task_1()
    }
    let cur: byte = Atomics.load(bytearr_i8, 0) as byte
    while (cur != N) {
        L.log("waiting, finished tasks: " + cur)
        Atomics.wait(arr_1, 0, cur)
        cur = Atomics.load(bytearr_i8, 0) as byte
    }

    for (let i = 0; i < N; i++) {
        launch task_2()
    }
    let cur_64: long = Atomics.load(bytearr_i64, 0) as long
    while (cur_64 != N) {
        L.log("waiting, finished tasks: " + cur_64)
        Atomics.wait(arr_2, 0, cur_64)
        cur_64 = Atomics.load(bytearr_i64, 0) as long
    }
    L.log("all tasks have finished")
}

function task_1(): Int {
    heavyComputation()
    L.log("Task finished. Notifying")
    Atomics.add(bytearr_i8, 0, 1 as byte)
    Atomics.notify(arr_1, 0)
    
    return 1
}

function task_2(): Int {
    heavyComputation()
    L.log("Task finished. Notifying")
    Atomics.add(bytearr_i64, 0, 1 as long)
    Atomics.notify(arr_2, 0)
    
    return 1
}

function heavyComputation(): Int {
    let i = 1
    for (let j = 0; j < 10000; j++) {
        i += j
    }
    return i
}
