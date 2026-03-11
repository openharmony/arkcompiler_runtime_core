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
  specified type. Array literal type inferred from assignment context.
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("ValueArray inference from assignment context")

    suite.addTest("ValueArray<int> reassigned from int literal array", () => {
        let a: ValueArray<int> = [0]
        a = [10, 20, 30]
        arktest.assertEQ(a[0], 10)
        arktest.assertEQ(a[1], 20)
        arktest.assertEQ(a[2], 30)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("ValueArray<double> reassigned from double literal array", () => {
        let a: ValueArray<double> = [0.0]
        a = [1.5, 2.5, 3.5]
        arktest.assertEQ(a[0], 1.5)
        arktest.assertEQ(a[1], 2.5)
        arktest.assertEQ(a[2], 3.5)
    })

    suite.addTest("ValueArray<boolean> reassigned from boolean literal array", () => {
        let a: ValueArray<boolean> = [false]
        a = [true, false]
        arktest.assertEQ(a[0], true)
        arktest.assertEQ(a[1], false)
    })

    return suite.run()
}
