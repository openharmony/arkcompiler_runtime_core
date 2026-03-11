/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
  If type used in a context is a value array type, and type of each expression
  is assignable to an array element type, then an array literal is of the
  specified type. Array literal type inferred from call parameter context.
---*/

function sumInts(arr: ValueArray<int>): int {
    let s: int = 0
    for (let i = 0; i < arr.length; i++) {
        s += arr[i]
    }
    return s
}

function sumDoubles(arr: ValueArray<double>): double {
    let s: double = 0.0
    for (let i = 0; i < arr.length; i++) {
        s += arr[i]
    }
    return s
}

function allTrue(arr: ValueArray<boolean>): boolean {
    for (let i = 0; i < arr.length; i++) {
        if (!arr[i]) return false
    }
    return true
}

function main(): int {
    let suite = new arktest.ArkTestsuite("ValueArray inference from call parameter context")

    suite.addTest("ValueArray<int> as function parameter", () => {
        let result = sumInts([10, 20, 30])
        arktest.assertEQ(result, 60)
    })

    suite.addTest("ValueArray<double> as function parameter", () => {
        let result = sumDoubles([1.5, 2.5])
        arktest.assertEQ(result, 4.0)
    })

    suite.addTest("ValueArray<double> with int widening as parameter", () => {
        let result = sumDoubles([1, 2, 3])
        arktest.assertEQ(result, 6.0)
    })

    suite.addTest("ValueArray<boolean> as function parameter", () => {
        arktest.assertTrue(allTrue([true, true, true]))
        arktest.assertTrue(!allTrue([true, false, true]))
    })

    return suite.run()
}
