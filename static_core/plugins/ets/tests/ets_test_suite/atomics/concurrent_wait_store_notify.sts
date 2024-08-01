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

// 1. If this test prints "1 ok" then the order of events is as follows: wait -> store -> notify
// 2. "0 not-equal": store -> wait -> notify, or store -> notify -> wait
// All other outputs are invalid

let buf: SharedArrayBuffer;
let arr: Int32Array;
let bytearr: Int8Array;

function main() {
    let c = new Console()

    buf = new SharedArrayBuffer(4);
    arr = new Int32Array(buf, 0, 1);
    bytearr = new Int8Array(buf, 0, 4)

    launch writer()

    let res = Atomics.wait(arr, 0, 0) as string
    c.println(res)
    assert(res == "ok" || res == "not-equal")
}

function writer(): Int {
    let c = new Console()

    wait()
    Atomics.store(bytearr, 0, 5 as byte)
    let count = Atomics.notify(arr, 0)
    c.println(count)
    return 1
}

function wait(): int {
    let i = 1
    for (let j = 0; j < 10000; j++) {
        i += j
    }
    return i
}
