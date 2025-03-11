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

let buf: SharedArrayBuffer;
let buf_2: SharedArrayBuffer;
let arr: Int8Array;
let arr_2: Int32Array;

function main() {
    buf = new SharedArrayBuffer(1);
    buf_2 = new SharedArrayBuffer(4);
    arr = new Int8Array(buf, 0, 1);
    arr_2 = new Int32Array(buf_2, 0, 1);

    for (let i = 0; i < 5; i++) {
        launch worker_i8(i as byte)
    }
    while (Atomics.load(arr, 0) != 5) {
    }
    L.log("Finished for Int8Array")

    for (let i = 0; i < 5; i++) {
        launch worker_i32(i as int)
    }
    while (Atomics.load(arr_2, 0) != 5) {
    }
    L.log("Finished for Int32Array")
}

function worker_i8(i: byte): Int {
    while (Atomics.compareExchange(arr, 0, i as byte, (i + 1) as byte) != i) {
    }
    return 1
}

function worker_i32(i: int): Int {
    while (Atomics.compareExchange(arr_2, 0, i as int, (i + 1) as int) != i) {
    }
    return 1
}