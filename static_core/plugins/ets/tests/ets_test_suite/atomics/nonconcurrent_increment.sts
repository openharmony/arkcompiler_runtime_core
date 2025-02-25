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

let N = 200
let RESULT = N as byte

function main() {
    let buf = new SharedArrayBuffer(4);
    let arr = new Int8Array(buf, 0, 1);

    for (let i = 0; i < N; i++) {
        Atomics.add(arr, 0, 1 as byte)
    }
    let x = Atomics.load(arr, 0)
    console.println("result is " + x)
    assertEQ(x, RESULT)
}
