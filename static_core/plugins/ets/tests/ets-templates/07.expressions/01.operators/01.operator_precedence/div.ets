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
desc: division operator with other operators
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Division operator precedence")

    suite.addTest("multiplication", () => {
        let res: int, t2 = 2, t3 = 3, t8 = 8

        res = t8 * t3 / t2
        arktest.assertEQ(res, 12)
    })

    suite.addTest("modulo", () => {
        let res: int, t2 = 2, t3 = 3, t8 = 8

        res = t8 % t3 / t2
        arktest.assertEQ(res, 1)
    })

    suite.addTest("addition", () => {
        let res: int, t2 = 2, t4 = 4

        res = t2 + t4 / t2
        arktest.assertEQ(res, 4)
    })

    suite.addTest("subtraction", () => {
        let res: int, t2 = 2, t4 = 4, t8 = 8

        res = t2 - t8 / t4
        arktest.assertEQ(res, 0)
    })

    suite.addTest("cast on result", () => {
        let res: int, t255 = 255, t510 = 510

        res = (t510 / t255).toByte()
        arktest.assertEQ(res, 2)
    })

    suite.addTest("cast on operand", () => {
        let res: int, t255 = 255, t510 = 510

        res = t510.toByte() / t255
        arktest.assertEQ(res, 0)
    })

    suite.addTest("left shift", () => {
        let res: int, t2 = 2, t4 = 4

        res = t4 << t4 / t2
        arktest.assertEQ(res, 16)
    })

    suite.addTest("right shift", () => {
        let res: int, t2 = 2, t4 = 4, t255 = 255

        res = t255 >> t4 / t2
        arktest.assertEQ(res, 63)
    })

    suite.addTest("unsigned right shift", () => {
        let res: int, t2 = 2, t8 = 8, t255 = 255

        res = - t255 >>> t8 / t2
        arktest.assertEQ(res, 268435440)
    })

    suite.addTest("relational <", () => {
        let t2 = 2, t255 = 255, t510 = 510
        arktest.assertFalse(t255 < t510 / t2)
    })

    suite.addTest("relational <=", () => {
        let t255 = 255, t510 = 510
        arktest.assertFalse(t255 <= t510 / t255)
    })

    suite.addTest("relational >", () => {
        let t2 = 2, t4 = 4, t8 = 8
        arktest.assertFalse(t2 > t8 / t4)
    })

    suite.addTest("relational >=", () => {
        let t4 = 4, t510 = 510
        arktest.assertFalse(t4 >= t510 / t4)
    })

    suite.addTest("equality ==", () => {
        let t2 = 2, t8 = 8
        arktest.assertFalse(t2 == t8 / t2)
    })

    suite.addTest("equality !=", () => {
        let t2 = 2, t4 = 4, t8 = 8
        arktest.assertFalse(t4 != t8 / t2)
    })

    suite.addTest("bitwise AND", () => {
        let res: int, t2 = 2, t3 = 3, t7 = 7

        res = t3 & t7 / t2
        arktest.assertEQ(res, 3)
    })

    suite.addTest("bitwise OR", () => {
        let res: int, t2 = 2, t7 = 7, t255 = 255

        res = t255 | t7 / t2
        arktest.assertEQ(res, 255)
    })

    suite.addTest("bitwise XOR", () => {
        let res: int, t2 = 2, t8 = 8, t255 = 255

        res = t255 ^ t8 / t2
        arktest.assertEQ(res, 251)
    })

    suite.addTest("ternary in branch", () => {
        let res: int, t2 = 2, t4 = 4, t8 = 8

        res = t8 > t2 ? t4 : t8 / t4
        arktest.assertEQ(res, 4)
    })

    suite.addTest("ternary in condition", () => {
        let res: int, t2 = 2, t4 = 4, t8 = 8

        res = t8 / t4 > t2 ? t4 : t8
        arktest.assertEQ(res, 8)
    })

    suite.addTest("assignment +=", () => {
        let res: int, t2 = 2, t8 = 8

        res = t2 += t8 / t2
        arktest.assertEQ(res, 6)
    })

    suite.addTest("assignment -=", () => {
        let res: int, t2 = 2, t4 = 4

        res = t2 -= t4 / t2
        arktest.assertEQ(res, 0)
    })

    suite.addTest("assignment *=", () => {
        let res: int, t2 = 2, t7 = 7, t8 = 8

        res = t8 *= t7 / t2
        arktest.assertEQ(res, 24)
    })

    suite.addTest("assignment %=", () => {
        let res: int, t2 = 2, t7 = 7, t8 = 8

        res = t8 %= t7 / t2
        arktest.assertEQ(res, 2)
    })

    suite.addTest("assignment &=", () => {
        let res: int, t2 = 2, t7 = 7, t255 = 255

        res = t7 &= t255 / t2
        arktest.assertEQ(res, 7)
    })

    suite.addTest("assignment |=", () => {
        let res: int, t4 = 4, t2 = 2, t7 = 7

        res = t2 |= t7 / t4
        arktest.assertEQ(res, 3)
    })

    suite.addTest("assignment ^=", () => {
        let res: int, t4 = 4, t8 = 8, t255 = 255

        res = t255 ^= t8 / t4
        arktest.assertEQ(res, 253)
    })

    suite.addTest("assignment <<=", () => {
        let res: int, t2 = 2, t3 = 3, t4 = 4

        res = t4 <<= t3 / t2
        arktest.assertEQ(res, 8)
    })

    suite.addTest("assignment >>=", () => {
        let res: int, t2 = 2, t3 = 3, t255 = 255

        res = t255 >>= t3 / t2
        arktest.assertEQ(res, 127)
    })

    suite.addTest("assignment >>>=", () => {
        let res: int, t2 = 2, t3 = 3, t255 = 255

        res = t255 >>>= t3 / t2
        arktest.assertEQ(res, 127)
    })

    return suite.run()
}
