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
    If any of operands are of type float, other operands are converted to type
    float before operator evaluation and the result type is float.
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Float promotion in constant expressions")

    suite.addTest("float + float produces float", () => {
        const r = 3.0f + 5.0f
        let f: float = r
        arktest.assertEQ(f, 8.0f)
    })

    suite.addTest("float * float produces float", () => {
        const r = 2.0f * 4.0f
        let f: float = r
        arktest.assertEQ(f, 8.0f)
    })

    suite.addTest("float - float produces float", () => {
        const r = 10.0f - 3.0f
        let f: float = r
        arktest.assertEQ(f, 7.0f)
    })

    suite.addTest("float / float produces float", () => {
        const r = 15.0f / 3.0f
        let f: float = r
        arktest.assertEQ(f, 5.0f)
    })

    suite.addTest("float + int literal promotes to float", () => {
        const r = 3.0f + 5
        let f: float = r
        arktest.assertEQ(f, 8.0f)
    })

    suite.addTest("int literal + float promotes to float", () => {
        const r = 5 + 3.0f
        let f: float = r
        arktest.assertEQ(f, 8.0f)
    })

    return suite.run()
}
