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
  If type used in a context is Any or Object, then Array Type Inference
  from Types of Elements is used.
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Array type inference from Any/Object context")

    suite.addTest("Object context: int array literal", () => {
        let a: Object = [1, 2, 3]
        arktest.assertTrue(a instanceof Array)
        arktest.assertEQ((a as int[])[0], 1)
        arktest.assertEQ((a as int[])[2], 3)
    })

    suite.addTest("Object context: string array literal", () => {
        let b: Object = ["hello", "world"]
        arktest.assertTrue(b instanceof Array)
        arktest.assertEQ((b as string[])[0], "hello")
        arktest.assertEQ((b as string[])[1], "world")
    })

    suite.addTest("Object context: double array literal", () => {
        let c: Object = [1.0, 2.0, 3.0]
        arktest.assertTrue(c instanceof Array)
        arktest.assertEQ((c as double[])[0], 1.0)
        arktest.assertEQ((c as double[])[2], 3.0)
    })

    suite.addTest("Any context: int array literal", () => {
        let d: Any = [1, 2, 3]
        arktest.assertTrue(d instanceof Array)
        arktest.assertEQ((d as int[])[0], 1)
        arktest.assertEQ((d as int[])[2], 3)
    })

    suite.addTest("Any context: string array literal", () => {
        let e: Any = ["hello", "world"]
        arktest.assertTrue(e instanceof Array)
        arktest.assertEQ((e as string[])[0], "hello")
        arktest.assertEQ((e as string[])[1], "world")
    })

    suite.addTest("Any context: double array literal", () => {
        let f: Any = [1.0, 2.0, 3.0]
        arktest.assertTrue(f instanceof Array)
        arktest.assertEQ((f as double[])[0], 1.0)
        arktest.assertEQ((f as double[])[2], 3.0)
    })

    return suite.run()
}
