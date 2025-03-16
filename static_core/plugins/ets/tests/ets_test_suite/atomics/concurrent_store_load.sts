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

let RESULT = 10 as long

let arr: BigInt64Array

function main() {
    let buf = new SharedArrayBuffer(8);
    arr = new BigInt64Array(buf, 0, 1);

    launch incrementer()

    while (Atomics.load(arr, 0) != RESULT) {
        wait()
    }
}

function incrementer(): Int {
    Atomics.store(arr, 0, RESULT)

    return 1
}

function wait(): int {
    let i = 1
    for (let j = 0; j < 10000; j++) {
        i += j
    }
    return i
}
