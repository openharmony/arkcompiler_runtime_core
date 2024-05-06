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

function test(byteToChange: int) {
    let buf = new SharedArrayBuffer(4);
    let arr = new Int32Array(buf, 0, 1);
    let bytearr = new Int8Array(buf, 0, 4);

    (new Console()).println("Testing for byte: " + byteToChange)
    Atomics.store(bytearr, byteToChange, 1);
    let x = Atomics.wait(arr, 0, 0)
    assert(x == "not-equal")
}

function main() {
    for (let i = 0; i < 4; i++) {
        test(i)
    }
}

