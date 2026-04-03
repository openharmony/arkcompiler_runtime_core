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
  specified type. Array literal type inferred from cast expression context.
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("ValueArray inference from cast context")

    suite.addTest("Cast array literal to ValueArray<int>", () => {
        let a = [1, 2, 3] as ValueArray<int>
        arktest.assertEQ(a[0], 1)
        arktest.assertEQ(a[1], 2)
        arktest.assertEQ(a[2], 3)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("Cast array literal to ValueArray<double>", () => {
        let a = [1.5, 2.5] as ValueArray<double>
        arktest.assertEQ(a[0], 1.5)
        arktest.assertEQ(a[1], 2.5)
    })

    suite.addTest("Cast array literal to ValueArray<boolean>", () => {
        let a = [true, false] as ValueArray<boolean>
        arktest.assertEQ(a[0], true)
        arktest.assertEQ(a[1], false)
    })

    return suite.run()
}
