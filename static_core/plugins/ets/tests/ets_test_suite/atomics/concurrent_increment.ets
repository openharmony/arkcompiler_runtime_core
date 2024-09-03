/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

let N = 200
let RESULT = N as byte

let arr: Int8Array
let arr_u32: Int32Array

function main() {
    let buf = new SharedArrayBuffer(4);
    let buf_2 = new SharedArrayBuffer(4);
    arr = new Int8Array(buf, 0, 1);
    arr_u32 = new Int32Array(buf_2, 0, 1);
    arr_u32[0] = N

    launch incrementer()
    while (Atomics.load(arr, 0) != RESULT) {
        wait()
    }

    launch decrementer()
    while (Atomics.load(arr_u32, 0) != 0) {
        wait()
    }
}

function incrementer(): Int {
    for (let i = 0; i < N; i++) {
        Atomics.add(arr, 0, 1 as byte)
    }

    return 1
}

function decrementer(): Int {
    for (let i = 0; i < N; i++) {
        Atomics.sub(arr_u32, 0, 1)
    }

    return 1
}

function wait(): int {
    let i = 1
    for (let j = 0; j < 10000; j++) {
        i += j
    }
    return i
}
