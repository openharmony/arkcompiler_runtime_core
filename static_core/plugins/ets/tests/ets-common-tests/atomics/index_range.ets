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

function testMain() {
    let arr: BigInt64Array = new BigInt64Array(2)
    Atomics.add(arr, 1, new BigInt(24))
    let res = Atomics.load(arr, 1).getLong()
    arktest.assertEQ(res, 24)

    let arrInt32: Int32Array = new Int32Array(3)
    Atomics.add(arrInt32, 2, 88)
    arktest.assertEQ(Atomics.load(arrInt32, 2), 88)
    arktest.assertEQ(arrInt32[2], 88)

    // Correct atomic loading after non-atomic store
    let arrUint32: Uint32Array = new Uint32Array(2)
    arrUint32[1] = 56
    arktest.assertEQ(Atomics.load(arrUint32,1), 56)

}

function main() {
    const suite = new arktest.ArkTestsuite('Function Atomics index_range tests')
    suite.addTest('Function testMain', testMain)
    return suite.run()
}