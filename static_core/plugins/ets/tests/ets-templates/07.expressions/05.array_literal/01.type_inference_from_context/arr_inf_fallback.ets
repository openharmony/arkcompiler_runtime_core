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
  When context type is not a fixed-size array, readonly array, or array type,
  the array literal type is inferred from element types.
---*/

class A_fallback {
    value: int = 11
}

function main(): int {
    let suite = new arktest.ArkTestsuite("arr_inf_fallback")

    suite.addTest("Object context, int element types inferred as int[]", () => {
        let o: Object = [1, 2]
        arktest.assertTrue(o instanceof Object)
        arktest.assertTrue(o instanceof Array)
        let arr = o as int[]
        arktest.assertEQ(arr[0], 1)
        arktest.assertEQ(arr[1], 2)
        arktest.assertEQ(arr.length, 2)
    })

    suite.addTest("Object context, string element types inferred as string[]", () => {
        let o: Object = ["hello", "world"]
        arktest.assertTrue(o instanceof Object)
        arktest.assertTrue(o instanceof Array)
        let arr = o as string[]
        arktest.assertEQ(arr[0], "hello")
        arktest.assertEQ(arr[1], "world")
        arktest.assertEQ(arr.length, 2)
    })

    suite.addTest("Object context, mixed primitive elements inferred as union[]", () => {
        let o: Object = [1, "hello", true]
        arktest.assertTrue(o instanceof Object)
        arktest.assertTrue(o instanceof Array)
        let arr = o as (int|string|boolean)[]
        arktest.assertEQ(arr[0] as int, 1)
        arktest.assertEQ(arr[1] as string, "hello")
        arktest.assertEQ(arr[2] as boolean, true)
        arktest.assertEQ(arr.length, 3)
    })

    suite.addTest("Object context, user-class instances inferred as A[]", () => {
        let o: Object = [new A_fallback(), new A_fallback()]
        arktest.assertTrue(o instanceof Object)
        arktest.assertTrue(o instanceof Array)
        let arr = o as A_fallback[]
        arktest.assertEQ(arr[0].value, 11)
        arktest.assertEQ(arr[1].value, 11)
        arktest.assertEQ(arr.length, 2)
    })

    suite.addTest("Object context, nested int arrays inferred as int[][]", () => {
        let o: Object = [[1, 2], [3, 4]]
        arktest.assertTrue(o instanceof Object)
        arktest.assertTrue(o instanceof Array)
        let arr = o as int[][]
        arktest.assertEQ(arr[0][0], 1)
        arktest.assertEQ(arr[0][1], 2)
        arktest.assertEQ(arr[1][0], 3)
        arktest.assertEQ(arr[1][1], 4)
        arktest.assertEQ(arr.length, 2)
    })

    suite.addTest("Object context, bigint elements inferred as bigint[]", () => {
        let o: Object = [1n, 2n, 3n]
        arktest.assertTrue(o instanceof Object)
        arktest.assertTrue(o instanceof Array)
        let arr = o as bigint[]
        arktest.assertEQ(arr[0], 1n)
        arktest.assertEQ(arr[1], 2n)
        arktest.assertEQ(arr[2], 3n)
        arktest.assertEQ(arr.length, 3)
    })

    suite.addTest("Object context, boolean elements inferred as boolean[]", () => {
        let o: Object = [true, false, true]
        arktest.assertTrue(o instanceof Object)
        arktest.assertTrue(o instanceof Array)
        let arr = o as boolean[]
        arktest.assertEQ(arr[0], true)
        arktest.assertEQ(arr[1], false)
        arktest.assertEQ(arr[2], true)
        arktest.assertEQ(arr.length, 3)
    })

    suite.addTest("Object context, boxed Number elements inferred as Number[]", () => {
        let o: Object = [new Number(1), new Number(2)]
        arktest.assertTrue(o instanceof Object)
        arktest.assertTrue(o instanceof Array)
        let arr = o as Number[]
        arktest.assertEQ(arr[0].valueOf(), 1)
        arktest.assertEQ(arr[1].valueOf(), 2)
        arktest.assertEQ(arr.length, 2)
    })

    return suite.run()
}
