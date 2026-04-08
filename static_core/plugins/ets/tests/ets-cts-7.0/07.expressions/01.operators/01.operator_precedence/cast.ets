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
desc: cast operator with other lower precedence operators
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Cast operator precedence")

    suite.addTest("left shift", () => {
        let res: int, t255 = 255, t257 = 257

        res = t255 << t257.toByte()
        arktest.assertEQ(res, 510)
    })

    suite.addTest("right shift", () => {
        let res: int, t257 = 257

        res = t257 >> t257.toByte()
        arktest.assertEQ(res, 128)
    })

    suite.addTest("unsigned right shift", () => {
        let res: int, t255 = 255, t257 = 257

        res = - t255 >>> t257.toByte()
        arktest.assertEQ(res, 2147483520)
    })

    suite.addTest("relational <", () => {
        let t2 = 2, t258 = 258
        arktest.assertFalse(t2 < t258.toByte())
    })

    suite.addTest("relational <=", () => {
        let t3 = 3, t258 = 258
        arktest.assertFalse(t3 <= t258.toByte())
    })

    suite.addTest("relational >", () => {
        let t2 = 2, t258 = 258
        arktest.assertFalse(t2 > t258.toByte())
    })

    suite.addTest("relational >=", () => {
        let t1 = 1, t258 = 258
        arktest.assertFalse(t1 >= t258.toByte())
    })

    suite.addTest("equality ==", () => {
        let t1 = 1, t258 = 258
        arktest.assertFalse(t1 == t258.toByte())
    })

    suite.addTest("equality !=", () => {
        let t2 = 2, t258 = 258
        arktest.assertFalse(t2 != t258.toByte())
    })

    suite.addTest("bitwise AND", () => {
        let res: int, t255 = 255, t258 = 258

        res = t255 & t258.toByte()
        arktest.assertEQ(res, 2)
    })

    suite.addTest("bitwise OR", () => {
        let res: int, t3 = 3, t258 = 258

        res = t3 | t258.toByte()
        arktest.assertEQ(res, 3)
    })

    suite.addTest("bitwise XOR", () => {
        let res: int, t255 = 255, t258 = 258

        res = t255 ^ t258.toByte()
        arktest.assertEQ(res, 253)
    })

    suite.addTest("ternary", () => {
        let res: int, t3 = 3, t4 = 4, t257 = 257, t258 = 258

        res = t3 <= t4 ? t257 : t258.toByte()
        arktest.assertEQ(res, 257)
    })

    suite.addTest("assignment +=", () => {
        let res: int, t257 = 257, t258 = 258

        res = t258 += t257.toByte()
        arktest.assertEQ(res, 259)
    })

    suite.addTest("assignment -=", () => {
        let res: int, t257 = 257, t258 = 258

        res = t258 -= t257.toByte()
        arktest.assertEQ(res, 257)
    })

    suite.addTest("assignment *=", () => {
        let res: int, t257 = 257, t258 = 258

        res = t258 *= t257.toByte()
        arktest.assertEQ(res, 258)
    })

    suite.addTest("assignment /=", () => {
        let res: int, t257 = 257, t258 = 258

        res = t258 /= t257.toByte()
        arktest.assertEQ(res, 258)
    })

    suite.addTest("assignment %=", () => {
        let res: int, t255 = 255, t258 = 258

        res = t255 %= t258.toByte()
        arktest.assertEQ(res, 1)
    })

    suite.addTest("assignment &=", () => {
        let res: int, t255 = 255, t258 = 258

        res = t255 &= t258.toByte()
        arktest.assertEQ(res, 2)
    })

    suite.addTest("assignment |=", () => {
        let res: int, t3 = 3, t258 = 258

        res = t3 |= t258.toByte()
        arktest.assertEQ(res, 3)
    })

    suite.addTest("assignment ^=", () => {
        let res: int, t255 = 255, t258 = 258

        res = t255 ^= t258.toByte()
        arktest.assertEQ(res, 253)
    })

    suite.addTest("assignment <<=", () => {
        let res: int, t255 = 255, t257 = 257

        res = t255 <<= t257.toByte()
        arktest.assertEQ(res, 510)
    })

    suite.addTest("assignment >>=", () => {
        let res: int, t257 = 257

        res = t257 >>= t257.toByte()
        arktest.assertEQ(res, 128)
    })

    suite.addTest("assignment >>>=", () => {
        let res: int, t255 = 255, t257 = 257

        res = t255 >>>= t257.toByte()
        arktest.assertEQ(res, 127)
    })

    return suite.run()
}
