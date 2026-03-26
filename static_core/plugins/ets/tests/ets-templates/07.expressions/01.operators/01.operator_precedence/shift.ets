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
desc: shift operators with other lower precedence operators
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Shift operators precedence")

    suite.addTest("shifts", () => {
        let res: int, t2 = 2, t3 = 3, t4 = 4, t255 = 255

        res = t255 << t3 >> t2 >>> t4
        arktest.assertEQ(res, 31)
    })

    suite.addTest("relational <", () => {
        let t2 = 2, t8 = 8
        arktest.assertFalse(t8 < t2 << t2)
    })

    suite.addTest("relational <=", () => {
        let t2 = 2, t255 = 255
        arktest.assertFalse(t255 <= t2 << t2)
    })

    suite.addTest("relational >", () => {
        let t2 = 2, t8 = 8
        arktest.assertFalse(t8 > t2 << t2)
    })

    suite.addTest("relational >=", () => {
        let t2 = 2, t4 = 4
        arktest.assertFalse(t4 >= t2 << t2)
    })

    suite.addTest("equality ==", () => {
        let t2 = 2, t4 = 4
        arktest.assertFalse(t4 == t2 << t2)
    })

    suite.addTest("equality !=", () => {
        let t2 = 2, t8 = 8
        arktest.assertFalse(t8 != t2 << t2)
    })

    suite.addTest("bitwise AND", () => {
        let res: int, t2 = 2, t4 = 4

        res = t4 & t4 << t2
        arktest.assertEQ(res, 0)
    })

    suite.addTest("bitwise OR", () => {
        let res: int, t2 = 2, t3 = 3, t4 = 4

        res = t3 | t4 << t2
        arktest.assertEQ(res, 19)
    })

    suite.addTest("bitwise XOR", () => {
        let res: int, t2 = 2, t8 = 8, t255 = 255

        res = t255 ^ t8 << t2
        arktest.assertEQ(res, 223)
    })

    suite.addTest("ternary in branch", () => {
        let res: int, t2 = 2, t3 = 3, t4 = 4

        res = t3 <= t4 ? t4 : t3 << t2
        arktest.assertEQ(res, 4)
    })

    suite.addTest("ternary in condition", () => {
        let res: int, t2 = 2, t3 = 3, t4 = 4

        res = t4 << t2 > t4 ? t4 : t3
        arktest.assertEQ(res, 4)
    })

    suite.addTest("assignment +=", () => {
        let res: int, t2 = 2, t4 = 4

        res = t2 += t4 << t2
        arktest.assertEQ(res, 18)
    })

    suite.addTest("assignment -=", () => {
        let res: int, t2 = 2, t4 = 4

        res = t2 -= t4 << t2
        arktest.assertEQ(res, -14)
    })

    suite.addTest("assignment *=", () => {
        let res: int, t2 = 2, t3 = 3, t8 = 8

        res = t8 *= t2 << t3
        arktest.assertEQ(res, 128)
    })

    suite.addTest("assignment /=", () => {
        let res: int, t2 = 2, t3 = 3, t255 = 255

        res = t255 /= t2 << t3
        arktest.assertEQ(res, 15)
    })

    suite.addTest("assignment %=", () => {
        let res: int, t2 = 2, t3 = 3, t255 = 255

        res = t255 %= t2 << t3
        arktest.assertEQ(res, 15)
    })

    suite.addTest("assignment &=", () => {
        let res: int, t3 = 3, t255 = 255

        res = t255 &= t255 << t3
        arktest.assertEQ(res, 248)
    })

    suite.addTest("assignment |=", () => {
        let res: int, t3 = 3, t255 = 255

        res = t255 |= t255 << t3
        arktest.assertEQ(res, 2047)
    })

    suite.addTest("assignment ^=", () => {
        let res: int, t4 = 4, t8 = 8, t255 = 255

        res = t255 ^= t4 << t8
        arktest.assertEQ(res, 1279)
    })

    suite.addTest("assignment <<=", () => {
        let res: int, t2 = 2, t3 = 3, t4 = 4

        res = t4 <<= t3 << t2
        arktest.assertEQ(res, 16384)
    })

    suite.addTest("assignment >>=", () => {
        let res: int, t2 = 2, t3 = 3, t255 = 255

        res = t255 >>= t2 << t3
        arktest.assertEQ(res, 0)
    })

    suite.addTest("assignment >>>=", () => {
        let res: int, t2 = 2, t3 = 3, t255 = 255

        res = t255 >>>= t3 << t2
        arktest.assertEQ(res, 0)
    })

    return suite.run()
}
