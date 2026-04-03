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
desc: postfix operators with other lower precedence operators
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Postfix operators precedence")

    suite.addTest("unary plus increment", () => {
        let res: int, t2 = 2
        res = + t2++
        arktest.assertEQ(res, 2)
        arktest.assertEQ(t2, 3)
    })

    suite.addTest("unary plus decrement", () => {
        let res: int, t2 = 2
        res = + t2--
        arktest.assertEQ(res, 2)
        arktest.assertEQ(t2, 1)
    })

    suite.addTest("unary minus increment", () => {
        let res: int, t2 = 2
        res = - t2++
        arktest.assertEQ(res, -2)
        arktest.assertEQ(t2, 3)
    })

    suite.addTest("unary minus decrement", () => {
        let res: int, t2 = 2
        res = - t2--
        arktest.assertEQ(res, -2)
        arktest.assertEQ(t2, 1)
    })

    suite.addTest("bitwise complement increment", () => {
        let res: int, t2 = 2
        res = ~ t2++
        arktest.assertEQ(res, -3)
        arktest.assertEQ(t2, 3)
    })

    suite.addTest("bitwise complement decrement", () => {
        let res: int, t2 = 2
        res = ~ t2--
        arktest.assertEQ(res, -3)
        arktest.assertEQ(t2, 1)
    })

    suite.addTest("multiplicative decrement", () => {
        let res: int, t2 = 2, t4 = 4
        res = t4-- * t2++
        arktest.assertEQ(res, 8)
        arktest.assertEQ(t2, 3)
        arktest.assertEQ(t4, 3)
    })

    suite.addTest("multiplicative increment", () => {
        let res: int, t2 = 2, t4 = 4
        res = t4++ * t2--
        arktest.assertEQ(res, 8)
        arktest.assertEQ(t2, 1)
        arktest.assertEQ(t4, 5)
    })

    suite.addTest("division decrement", () => {
        let res: int, t2 = 2, t4 = 4
        res = t4-- / t2++
        arktest.assertEQ(res, 2)
        arktest.assertEQ(t2, 3)
        arktest.assertEQ(t4, 3)
    })

    suite.addTest("division increment", () => {
        let res: int, t2 = 2, t4 = 4
        res = t4++ / t2--
        arktest.assertEQ(res, 2)
        arktest.assertEQ(t2, 1)
        arktest.assertEQ(t4, 5)
    })

    suite.addTest("modulo decrement", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255-- % t4++
        arktest.assertEQ(res, 3)
        arktest.assertEQ(t255, 254)
        arktest.assertEQ(t4, 5)
    })

    suite.addTest("modulo increment", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255++ % t4--
        arktest.assertEQ(res, 3)
        arktest.assertEQ(t255, 256)
        arktest.assertEQ(t4, 3)
    })

    suite.addTest("addition decrement", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255-- + t4++
        arktest.assertEQ(res, 259)
        arktest.assertEQ(t255, 254)
        arktest.assertEQ(t4, 5)
    })

    suite.addTest("addition increment", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255++ + t4--
        arktest.assertEQ(res, 259)
        arktest.assertEQ(t255, 256)
        arktest.assertEQ(t4, 3)
    })

    suite.addTest("subtraction decrement", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255-- - t4++
        arktest.assertEQ(res, 251)
        arktest.assertEQ(t255, 254)
        arktest.assertEQ(t4, 5)
    })

    suite.addTest("subtraction increment", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255++ - t4--
        arktest.assertEQ(res, 251)
        arktest.assertEQ(t255, 256)
        arktest.assertEQ(t4, 3)
    })

    suite.addTest("casting decrement", () => {
        let res: int, t4 = 4, t255 = 255
        res = (t255--).toByte() - (t4++).toByte()
        arktest.assertEQ(res, -5)
        arktest.assertEQ(t255, 254)
        arktest.assertEQ(t4, 5)
    })

    suite.addTest("casting increment", () => {
        let res: int, t4 = 4, t255 = 255
        res = (t255++).toByte() - (t4--).toByte()
        arktest.assertEQ(res, -5)
        arktest.assertEQ(t255, 256)
        arktest.assertEQ(t4, 3)
    })

    suite.addTest("left shift decrement", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255-- << t4++
        arktest.assertEQ(res, 4080)
        arktest.assertEQ(t255, 254)
        arktest.assertEQ(t4, 5)
    })

    suite.addTest("left shift increment", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255++ << t4--
        arktest.assertEQ(res, 4080)
        arktest.assertEQ(t255, 256)
        arktest.assertEQ(t4, 3)
    })

    suite.addTest("right shift decrement", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255-- >> t4++
        arktest.assertEQ(res, 15)
        arktest.assertEQ(t255, 254)
        arktest.assertEQ(t4, 5)
    })

    suite.addTest("right shift increment", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255++ >> t4--
        arktest.assertEQ(res, 15)
        arktest.assertEQ(t255, 256)
        arktest.assertEQ(t4, 3)
    })

    suite.addTest("right no-sign shift decrement", () => {
        let res: int, t4 = 4, t255 = 255
        res = - t255-- >>> t4++
        arktest.assertEQ(res, 268435440)
        arktest.assertEQ(t255, 254)
        arktest.assertEQ(t4, 5)
    })

    suite.addTest("right no-sign shift increment", () => {
        let res: int, t4 = 4, t255 = 255
        res = - t255++ >>> t4--
        arktest.assertEQ(res, 268435440)
        arktest.assertEQ(t255, 256)
        arktest.assertEQ(t4, 3)
    })

    suite.addTest("relational <", () => {
        let t2 = 2, t3 = 3
        arktest.assertFalse(t3-- < t2++)
    })

    suite.addTest("relational <=", () => {
        let t4 = 4, t8 = 8
        arktest.assertFalse(t8-- <= t4++)
    })

    suite.addTest("relational >", () => {
        let t2 = 2, t3 = 3
        arktest.assertFalse(t2++ > t3--)
    })

    suite.addTest("relational >=", () => {
        let t4 = 4, t8 = 8
        arktest.assertFalse(t4++ >= t8--)
    })

    suite.addTest("equality == decrement", () => {
        let t2 = 2, t3 = 3
        arktest.assertFalse(t3-- == t2)
    })

    suite.addTest("equality == increment", () => {
        let t3 = 3, t4 = 4
        arktest.assertFalse(t3++ == t4)
    })

    suite.addTest("equality != decrement", () => {
        let t3 = 3
        arktest.assertFalse(t3-- != 3)
    })

    suite.addTest("equality != increment", () => {
        let t3 = 3
        arktest.assertFalse(t3++ != 3)
    })

    suite.addTest("bitwise AND decrement", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255-- & t4++
        arktest.assertEQ(res, 4)
        arktest.assertEQ(t255, 254)
        arktest.assertEQ(t4, 5)
    })

    suite.addTest("bitwise AND increment", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255++ & t4--
        arktest.assertEQ(res, 4)
        arktest.assertEQ(t255, 256)
        arktest.assertEQ(t4, 3)
    })

    suite.addTest("bitwise OR decrement", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255-- | t4++
        arktest.assertEQ(res, 255)
        arktest.assertEQ(t255, 254)
        arktest.assertEQ(t4, 5)
    })

    suite.addTest("bitwise OR increment", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255++ | t4--
        arktest.assertEQ(res, 255)
        arktest.assertEQ(t255, 256)
        arktest.assertEQ(t4, 3)
    })

    suite.addTest("bitwise XOR decrement", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255-- ^ t4++
        arktest.assertEQ(res, 251)
        arktest.assertEQ(t255, 254)
        arktest.assertEQ(t4, 5)
    })

    suite.addTest("bitwise XOR increment", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255++ ^ t4--
        arktest.assertEQ(res, 251)
        arktest.assertEQ(t255, 256)
        arktest.assertEQ(t4, 3)
    })

    suite.addTest("ternary decrement", () => {
        let res: int, t4 = 4, t8 = 8, t255 = 255
        res = t8 > t4 ? t255-- : t4++
        arktest.assertEQ(res, 255)
        arktest.assertEQ(t255, 254)
        arktest.assertEQ(t4, 4)
    })

    suite.addTest("ternary increment", () => {
        let res: int, t4 = 4, t8 = 8, t255 = 255
        res = t8 > t4 ? t4-- : t255++
        arktest.assertEQ(res, 4)
        arktest.assertEQ(t255, 255)
        arktest.assertEQ(t4, 3)
    })

    suite.addTest("simple assignment decrement", () => {
        let res: int, t3 = 3, t255 = 255
        res = t255 = t3--
        arktest.assertEQ(res, 3)
        arktest.assertEQ(t255, 3)
        arktest.assertEQ(t3, 2)
    })

    suite.addTest("simple assignment increment", () => {
        let res: int, t3 = 3, t255 = 255
        res = t255 = t3++
        arktest.assertEQ(res, 3)
        arktest.assertEQ(t255, 3)
        arktest.assertEQ(t3, 4)
    })

    suite.addTest("assignment += decrement", () => {
        let res: int, t3 = 3, t255 = 255
        res = t255 += t3--
        arktest.assertEQ(res, 258)
        arktest.assertEQ(t255, 258)
        arktest.assertEQ(t3, 2)
    })

    suite.addTest("assignment += increment", () => {
        let res: int, t3 = 3, t255 = 255
        res = t255 += t3++
        arktest.assertEQ(res, 258)
        arktest.assertEQ(t255, 258)
        arktest.assertEQ(t3, 4)
    })

    suite.addTest("assignment -= decrement", () => {
        let res: int, t3 = 3, t255 = 255
        res = t255 -= t3--
        arktest.assertEQ(res, 252)
        arktest.assertEQ(t255, 252)
        arktest.assertEQ(t3, 2)
    })

    suite.addTest("assignment -= increment", () => {
        let res: int, t3 = 3, t255 = 255
        res = t255 -= t3++
        arktest.assertEQ(res, 252)
        arktest.assertEQ(t255, 252)
        arktest.assertEQ(t3, 4)
    })

    suite.addTest("assignment *= decrement", () => {
        let res: int, t3 = 3, t255 = 255
        res = t255 *= t3--
        arktest.assertEQ(res, 765)
        arktest.assertEQ(t255, 765)
        arktest.assertEQ(t3, 2)
    })

    suite.addTest("assignment *= increment", () => {
        let res: int, t3 = 3, t255 = 255
        res = t255 *= t3++
        arktest.assertEQ(res, 765)
        arktest.assertEQ(t255, 765)
        arktest.assertEQ(t3, 4)
    })

    suite.addTest("assignment /= decrement", () => {
        let res: int, t3 = 3, t255 = 255
        res = t255 /= t3--
        arktest.assertEQ(res, 85)
        arktest.assertEQ(t255, 85)
        arktest.assertEQ(t3, 2)
    })

    suite.addTest("assignment /= increment", () => {
        let res: int, t3 = 3, t255 = 255
        res = t255 /= t3++
        arktest.assertEQ(res, 85)
        arktest.assertEQ(t255, 85)
        arktest.assertEQ(t3, 4)
    })

    suite.addTest("assignment %= decrement", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255 %= t4--
        arktest.assertEQ(res, 3)
        arktest.assertEQ(t255, 3)
        arktest.assertEQ(t4, 3)
    })

    suite.addTest("assignment %= increment", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255 %= t4++
        arktest.assertEQ(res, 3)
        arktest.assertEQ(t255, 3)
        arktest.assertEQ(t4, 5)
    })

    suite.addTest("assignment <<= decrement", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255 <<= t4--
        arktest.assertEQ(res, 4080)
        arktest.assertEQ(t255, 4080)
        arktest.assertEQ(t4, 3)
    })

    suite.addTest("assignment <<= increment", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255 <<= t4++
        arktest.assertEQ(res, 4080)
        arktest.assertEQ(t255, 4080)
        arktest.assertEQ(t4, 5)
    })

    suite.addTest("assignment >>= decrement", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255 >>= t4--
        arktest.assertEQ(res, 15)
        arktest.assertEQ(t255, 15)
        arktest.assertEQ(t4, 3)
    })

    suite.addTest("assignment >>= increment", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255 >>= t4++
        arktest.assertEQ(res, 15)
        arktest.assertEQ(t255, 15)
        arktest.assertEQ(t4, 5)
    })

    suite.addTest("assignment >>>= decrement", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255 >>>= t4--
        arktest.assertEQ(res, 15)
        arktest.assertEQ(t255, 15)
        arktest.assertEQ(t4, 3)
    })

    suite.addTest("assignment >>>= increment", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255 >>>= t4++
        arktest.assertEQ(res, 15)
        arktest.assertEQ(t255, 15)
        arktest.assertEQ(t4, 5)
    })

    suite.addTest("assignment &= decrement", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255 &= t4--
        arktest.assertEQ(res, 4)
        arktest.assertEQ(t255, 4)
        arktest.assertEQ(t4, 3)
    })

    suite.addTest("assignment &= increment", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255 &= t4++
        arktest.assertEQ(res, 4)
        arktest.assertEQ(t255, 4)
        arktest.assertEQ(t4, 5)
    })

    suite.addTest("assignment |= decrement", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255 |= t4--
        arktest.assertEQ(res, 255)
        arktest.assertEQ(t255, 255)
        arktest.assertEQ(t4, 3)
    })

    suite.addTest("assignment |= increment", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255 |= t4++
        arktest.assertEQ(res, 255)
        arktest.assertEQ(t255, 255)
        arktest.assertEQ(t4, 5)
    })

    suite.addTest("assignment ^= decrement", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255 ^= t4--
        arktest.assertEQ(res, 251)
        arktest.assertEQ(t255, 251)
        arktest.assertEQ(t4, 3)
    })

    suite.addTest("assignment ^= increment", () => {
        let res: int, t4 = 4, t255 = 255
        res = t255 ^= t4++
        arktest.assertEQ(res, 251)
        arktest.assertEQ(t255, 251)
        arktest.assertEQ(t4, 5)
    })

    return suite.run()
}
