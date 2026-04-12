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
    If all operands are integers, they are converted to a big integer type that
    allows handling arbitrary-precision integers. A single integer literal or a
    constant of an integer type is also converted to the big integer type.
    Type Inference for Constant Expressions is always used to infer the type.
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Integer big integer type in constant expressions")

    suite.addTest("int + int can be assigned to int", () => {
        const r = 3 + 5
        let i: int = r
        arktest.assertEQ(i, 8)
    })

    suite.addTest("int + int can be assigned to long", () => {
        const r = 3 + 5
        let l: long = r
        arktest.assertEQ(l, 8 as long)
    })

    suite.addTest("int + int can be assigned to byte if fits", () => {
        const r = 3 + 5
        let b: byte = r
        arktest.assertEQ(b, (8).toByte())
    })

    suite.addTest("int + int can be assigned to short if fits", () => {
        const r = 3 + 5
        let s: short = r
        arktest.assertEQ(s, (8).toShort())
    })

    suite.addTest("int * int stays integer", () => {
        const r = 10 * 2
        let i: int = r
        arktest.assertEQ(i, 20)
    })

    suite.addTest("single integer literal can be assigned to various types", () => {
        const r = 7
        let i: int = r
        let l: long = r
        let b: byte = r
        let s: short = r
        arktest.assertEQ(i, 7)
        arktest.assertEQ(l, 7 as long)
        arktest.assertEQ(b, (7).toByte())
        arktest.assertEQ(s, (7).toShort())
    })

    suite.addTest("unary minus on integer stays integer", () => {
        const r = -10
        let i: int = r
        arktest.assertEQ(i, -10)
    })

    suite.addTest("bitwise complement on integer stays integer", () => {
        const r = ~15
        let i: int = r
        arktest.assertEQ(i, -16)
    })

    suite.addTest("shift on integers stays integer", () => {
        const r = 1 << 3
        let i: int = r
        arktest.assertEQ(i, 8)
    })

    suite.addTest("modulo on integers stays integer", () => {
        const r = 17 % 5
        let i: int = r
        arktest.assertEQ(i, 2)
    })

    return suite.run()
}
