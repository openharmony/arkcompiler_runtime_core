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

function main(): int {
    let arrayTestsuite = new ArkTestsuite("arrayTestsuite")
    arrayTestsuite.addTest("SortWithUndefined", SortWithUndefined)
    arrayTestsuite.addTest("SortWithUndefinedAndNull", SortWithUndefinedAndNull)
    return arrayTestsuite.run()
}

function SortWithUndefined() {
    let myComparefn_1 = (x: number | undefined, y: number | undefined) => {
        if (x == undefined) {
            return -1.
        }
        if (y == undefined) {
            return 1.
        }
        return 0.
    }
    let x: Array<number | undefined> = new Array<number | undefined>(undefined, 1., undefined, 5., 2., 3.)
    x.sort(myComparefn_1)
    assertEQ(x.length, 6)
    assertEQ(x[0], 1)
    assertEQ(x[1], 5)
    assertEQ(x[2], 2)
    assertEQ(x[3], 3)
    assertEQ(x[4], undefined)
    assertEQ(x[5], undefined)
    let y: Array<int | undefined> = new Array<int | undefined>(3, undefined, 1, undefined, 5, 2)
    let myComparefn_2 = (x: int | undefined, y: int | undefined) => {
        if (x == undefined) {
            return 1.
        }
        if (y == undefined) {
            return -1.
        }
        return 0.
    }
    y.sort(myComparefn_2)
    assertEQ(y.length, 6)
    assertEQ(y[0], 3)
    assertEQ(y[1], 1)
    assertEQ(y[2], 5)
    assertEQ(y[3], 2)
    assertEQ(y[4], undefined)
    assertEQ(y[5], undefined)
}

function SortWithUndefinedAndNull() {
    let arr: Array<number | undefined | null> = new Array<number | undefined | null>(1., 1000., 3., null, undefined, -2., undefined, 0.)
    arr.sort()
    assertEQ(arr.length, 8)
    assertEQ(arr[0], -2)
    assertEQ(arr[1], 0)
    assertEQ(arr[2], 1)
    assertEQ(arr[3], 1000)
    assertEQ(arr[4], 3)
    assertEQ(arr[5], null)
    assertEQ(arr[6], undefined)
    assertEQ(arr[7], undefined)
}
