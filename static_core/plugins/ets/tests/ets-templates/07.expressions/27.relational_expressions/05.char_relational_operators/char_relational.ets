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
    A comparison of two operands of type char is performed as an integer
    comparison of two unsigned 16-bit values.
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Character relational operators")

    suite.addTest("less than with lowercase letters", () => {
        let a: char = c'a'
        let b: char = c'b'
        arktest.assertTrue(a < b)
        arktest.assertFalse(b < a)
    })

    suite.addTest("less than with uppercase letters", () => {
        let a: char = c'A'
        let z: char = c'Z'
        arktest.assertTrue(a < z)
        arktest.assertFalse(z < a)
    })

    suite.addTest("less than or equal with equal chars", () => {
        let a: char = c'x'
        let b: char = c'x'
        arktest.assertTrue(a <= b)
        arktest.assertTrue(b <= a)
    })

    suite.addTest("less than or equal with different chars", () => {
        let a: char = c'0'
        let b: char = c'9'
        arktest.assertTrue(a <= b)
        arktest.assertFalse(b <= a)
    })

    suite.addTest("greater than with digits", () => {
        let nine: char = c'9'
        let zero: char = c'0'
        arktest.assertTrue(nine > zero)
        arktest.assertFalse(zero > nine)
    })

    suite.addTest("greater than or equal with equal chars", () => {
        let a: char = c'M'
        let b: char = c'M'
        arktest.assertTrue(a >= b)
        arktest.assertTrue(b >= a)
    })

    suite.addTest("greater than or equal with different chars", () => {
        let a: char = c'z'
        let b: char = c'a'
        arktest.assertTrue(a >= b)
        arktest.assertFalse(b >= a)
    })

    suite.addTest("uppercase has lower code point than lowercase", () => {
        let upper: char = c'A'
        let lower: char = c'a'
        arktest.assertTrue(upper < lower)
        arktest.assertTrue(lower > upper)
    })

    suite.addTest("space has lower code point than printable chars", () => {
        let space: char = c' '
        let excl: char = c'!'
        arktest.assertTrue(space < excl)
    })

    suite.addTest("comparison is consistent across all four operators", () => {
        let a: char = c'C'
        let b: char = c'D'
        arktest.assertTrue(a < b)
        arktest.assertTrue(a <= b)
        arktest.assertFalse(a > b)
        arktest.assertFalse(a >= b)
        arktest.assertTrue(b > a)
        arktest.assertTrue(b >= a)
        arktest.assertFalse(b < a)
        arktest.assertFalse(b <= a)
    })

    return suite.run()
}
