/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

const BYTE_LENGTH = 128

let ab = new ArrayBuffer(BYTE_LENGTH)
let int8array = new Int8Array(ab)
let ab2 = new ArrayBuffer(BYTE_LENGTH)
let int16array = new Int16Array(ab2)
let ab3 = new ArrayBuffer(BYTE_LENGTH)
let int32array = new Int32Array(ab3)
let ab4 = new ArrayBuffer(BYTE_LENGTH)
let uint16array = new Uint16Array(ab4)
let ab5 = new ArrayBuffer(BYTE_LENGTH)
let uint32array = new Uint32Array(ab5)

let typeArrayList = [
    int8array,
    int16array,
    int32array,
    uint32array
]

function testSub() {
    for (let i = 0; i < typeArrayList.length; i++) {
        let ta = typeArrayList[i]
        let arraySize = ta.length

        for (let j = 0; j < arraySize; j++) {
            Atomics.store(ta, j, 48)

            let old = Atomics.sub(ta, j, 12)
            assertEQ(old, 48)

            for (let k = 0; k < arraySize; k++) {
                let re = (k === j) ? 36 : 0
                assertEQ(Atomics.load(ta, k), re)
            }

            Atomics.store(ta, j, 0)
        }
    }
}

function main() {
    const suite = new ArkTestsuite('Function Atomics baseOperator tests')
    suite.addTest('Function testSub', testSub)
    return suite.run()
}