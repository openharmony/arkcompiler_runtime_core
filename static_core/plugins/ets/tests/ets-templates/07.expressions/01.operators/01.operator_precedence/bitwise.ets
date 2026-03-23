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
desc: bitwise operators with other operators
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Bitwise operators precedence")

    suite.addTest("bitwise OR over AND", () => {
        let res: int, t3 = 3, t4 = 4, t8 = 8

        res = t3 | t4 & t8
        arktest.assertEQ(res, 3)
    })

    suite.addTest("bitwise XOR over AND", () => {
        let res: int, t2 = 2, t3 = 3, t4 = 4

        res = t3 ^ t4 & t2
        arktest.assertEQ(res, 3)
    })

    suite.addTest("bitwise OR over XOR", () => {
        let res: int, t2 = 2, t3 = 3, t4 = 4

        res = t3 | t4 ^ t2
        arktest.assertEQ(res, 7)
    })

    suite.addTest("ternary AND", () => {
        let res: int, t2 = 2, t3 = 3, t4 = 4, t7 = 7

        res = t3 <= t4 ? t7 : t2 & t3
        arktest.assertEQ(res, 7)
    })

    suite.addTest("ternary XOR", () => {
        let res: int, t2 = 2, t3 = 3, t4 = 4, t7 = 7

        res = t3 <= t4 ? t7 : t2 ^ t3
        arktest.assertEQ(res, 7)
    })

    suite.addTest("ternary OR", () => {
        let res: int, t2 = 2, t3 = 3, t4 = 4, t7 = 7, t255 = 255

        res = t3 <= t4 ? t7 : t2 | t255
        arktest.assertEQ(res, 7)
    })

    suite.addTest("assignment += AND", () => {
        let res: int, t2 = 2, t3 = 3, t7 = 7

        res = t2 += t7 & t3
        arktest.assertEQ(res, 5)
    })

    suite.addTest("assignment += XOR", () => {
        let res: int, t2 = 2, t3 = 3, t7 = 7

        res = t2 += t7 ^ t3
        arktest.assertEQ(res, 6)
    })

    suite.addTest("assignment += OR", () => {
        let res: int, t2 = 2, t3 = 3, t7 = 7

        res = t2 += t7 | t3
        arktest.assertEQ(res, 9)
    })

    suite.addTest("assignment -= AND", () => {
        let res: int, t2 = 2, t3 = 3, t7 = 7

        res = t2 -= t7 & t3
        arktest.assertEQ(res, -1)
    })

    suite.addTest("assignment -= XOR", () => {
        let res: int, t2 = 2, t3 = 3, t7 = 7

        res = t2 -= t7 ^ t3
        arktest.assertEQ(res, -2)
    })

    suite.addTest("assignment -= OR", () => {
        let res: int, t2 = 2, t7 = 7, t255 = 255

        res = t2 -= t7 | t255
        arktest.assertEQ(res, -253)
    })

    suite.addTest("assignment *= AND", () => {
        let res: int, t2 = 2, t3 = 3, t7 = 7

        res = t2 *= t7 & t3
        arktest.assertEQ(res, 6)
    })

    suite.addTest("assignment *= XOR", () => {
        let res: int, t2 = 2, t3 = 3, t7 = 7

        res = t2 *= t7 ^ t3
        arktest.assertEQ(res, 8)
    })

    suite.addTest("assignment *= OR", () => {
        let res: int, t2 = 2, t7 = 7, t255 = 255

        res = t2 *= t7 | t255
        arktest.assertEQ(res, 510)
    })

    suite.addTest("assignment /= AND", () => {
        let res: int, t3 = 3, t7 = 7, t255 = 255

        res = t255 /= t7 & t3
        arktest.assertEQ(res, 85)
    })

    suite.addTest("assignment /= XOR", () => {
        let res: int, t3 = 3, t7 = 7, t255 = 255

        res = t255 /= t7 ^ t3
        arktest.assertEQ(res, 63)
    })

    suite.addTest("assignment /= OR", () => {
        let res: int, t7 = 7, t255 = 255

        res = t255 /= t7 | t255
        arktest.assertEQ(res, 1)
    })

    suite.addTest("assignment %= AND", () => {
        let res: int, t3 = 3, t7 = 7, t255 = 255

        res = t255 %= t7 & t3
        arktest.assertEQ(res, 0)
    })

    suite.addTest("assignment %= XOR", () => {
        let res: int, t3 = 3, t7 = 7, t255 = 255

        res = t255 %= t7 ^ t3
        arktest.assertEQ(res, 3)
    })

    suite.addTest("assignment %= OR", () => {
        let res: int, t7 = 7, t255 = 255

        res = t255 %= t7 | t255
        arktest.assertEQ(res, 0)
    })

    suite.addTest("assignment &= AND", () => {
        let res: int, t3 = 3, t7 = 7, t255 = 255

        res = t255 &= t7 & t3
        arktest.assertEQ(res, 3)
    })

    suite.addTest("assignment &= XOR", () => {
        let res: int, t7 = 7, t255 = 255

        res = t255 &= t7 ^ t255
        arktest.assertEQ(res, 248)
    })

    suite.addTest("assignment &= OR", () => {
        let res: int, t7 = 7, t255 = 255

        res = t255 &= t7 | t255
        arktest.assertEQ(res, 255)
    })

    suite.addTest("assignment ^= AND", () => {
        let res: int, t3 = 3, t7 = 7, t255 = 255

        res = t255 ^= t7 & t3
        arktest.assertEQ(res, 252)
    })

    suite.addTest("assignment ^= XOR", () => {
        let res: int, t7 = 7, t255 = 255

        res = t255 ^= t7 ^ t255
        arktest.assertEQ(res, 7)
    })

    suite.addTest("assignment ^= OR", () => {
        let res: int, t7 = 7, t255 = 255

        res = t255 ^= t7 | t255
        arktest.assertEQ(res, 0)
    })

    suite.addTest("assignment |= AND", () => {
        let res: int, t3 = 3, t7 = 7, t255 = 255

        res = t255 |= t7 & t3
        arktest.assertEQ(res, 255)
    })

    suite.addTest("assignment |= XOR", () => {
        let res: int, t7 = 7, t255 = 255

        res = t255 |= t7 ^ t255
        arktest.assertEQ(res, 255)
    })

    suite.addTest("assignment |= OR", () => {
        let res: int, t7 = 7, t255 = 255

        res = t255 |= t7 | t255
        arktest.assertEQ(res, 255)
    })

    suite.addTest("assignment <<= AND", () => {
        let res: int, t3 = 3, t7 = 7, t255 = 255

        res = t255 <<= t7 & t3
        arktest.assertEQ(res, 2040)
    })

    suite.addTest("assignment <<= XOR", () => {
        let res: int, t7 = 7, t255 = 255

        res = t255 <<= t7 ^ t255
        arktest.assertEQ(res, -16777216)
    })

    suite.addTest("assignment <<= OR", () => {
        let res: int, t7 = 7, t255 = 255

        res = t255 <<= t7 | t255
        arktest.assertEQ(res, -2147483648)
    })

    suite.addTest("assignment >>= AND", () => {
        let res: int, t3 = 3, t7 = 7, t255 = 255

        res = t255 >>= t7 & t3
        arktest.assertEQ(res, 31)
    })

    suite.addTest("assignment >>= XOR", () => {
        let res: int, t2 = 2, t7 = 7, t255 = 255

        res = t255 >>= t2 ^ t7
        arktest.assertEQ(res, 7)
    })

    suite.addTest("assignment >>= OR", () => {
        let res: int, t7 = 7, t255 = 255

        res = t255 >>= t7 | t7
        arktest.assertEQ(res, 1)
    })

    suite.addTest("assignment >>>= AND", () => {
        let res: int, t3 = 3, t7 = 7, t255 = 255

        res = t255 >>>= t7 & t3
        arktest.assertEQ(res, 31)
    })

    suite.addTest("assignment >>>= XOR", () => {
        let res: int, t2 = 2, t7 = 7, t255 = 255

        res = t255 >>>= t2 ^ t7
        arktest.assertEQ(res, 7)
    })

    suite.addTest("assignment >>>= OR", () => {
        let res: int, t7 = 7, t255 = 255

        res = t255 >>>= t7 | t7
        arktest.assertEQ(res, 1)
    })

    return suite.run()
}
