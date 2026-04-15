/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

/*---
desc: >-
  Initializer expressions are executed from left to right.
  The n'th expression specifies the value of the n-1'th element of the array.
---*/

let arr: int[] = new int[4](0)
function getFromArr(cnt: int): int {
    return arr[cnt]
}

function throwIfTrue(p: boolean): boolean {
    if (p) {
        throw new Error()
    }
    return p
}

function main(): int {
    let suite = new arktest.ArkTestsuite("arr_abrupt")

    suite.addTest("abrupt completion from out-of-bounds index", () => {
        let cnt: int = 0
        let a: int[] = [ 42 ]
        try {
            a = [getFromArr(cnt++), getFromArr(cnt++), getFromArr(cnt++), getFromArr(cnt++), getFromArr(cnt++)]
        } catch (e) {
            arktest.assertTrue( a.length == 1 && a[0] == 42 )
            return
        }
        arktest.assertTrue( false )
    })

    suite.addTest("abrupt completion from thrown Error", () => {
        let a: boolean[] = []
        try {
            a = [throwIfTrue(false), throwIfTrue(false), throwIfTrue(true), throwIfTrue(false), throwIfTrue(false)]
        } catch (e) {
            arktest.assertTrue( a.length == 0 )
            return
        }
        arktest.assertTrue( false )
    })

    return suite.run()
}
