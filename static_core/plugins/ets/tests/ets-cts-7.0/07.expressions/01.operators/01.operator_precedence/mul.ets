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
desc: multiplicative operator with other operators
---*/

function mulDiv(a: int, b: int, c: int): int { return a / b * c }
function mulMod(a: int, b: int, c: int): int { return a % b * c }
function mulAdd(a: int, b: int, c: int): int { return a + b * c }
function mulSub(a: int, b: int, c: int): int { return a - b * c }
function mulCastRes(a: int, b: int): int { return a * b.toByte() }
function mulCastOp(a: int, b: int): int { return a.toByte() * b }
function mulShl(a: int, b: int, c: int): int { return a << b * c }
function mulShrNeg(a: int, b: int, c: int): int { return -a >> b * c }
function mulUshrNeg(a: int, b: int, c: int): int { return -a >>> b * c }
function mulLt(a: int, b: int, c: int): boolean { return a < b * c }
function mulLe(a: int, b: int, c: int): boolean { return a <= b * c }
function mulGt(a: int, b: int, c: int): boolean { return a > b * c }
function mulGe(a: int, b: int, c: int): boolean { return a >= b * c }
function mulEqEq(a: int, b: int, c: int): boolean { return a == b * c }
function mulNeq(a: int, b: int, c: int): boolean { return a != b * c }
function mulBitAnd(a: int, b: int, c: int): int { return a & b * c }
function mulBitOr(a: int, b: int, c: int): int { return a | b * c }
function mulBitXor(a: int, b: int, c: int): int { return a ^ b * c }
function mulTernBranch(a: int, b: int, c: int): int { return a <= b ? b : a * b }
function mulTernCond(a: int, b: int, c: int, d: int): int { return a * b > c ? c : d }
function mulAssignAdd(a: int, b: int, c: int): int { return a += b * c }
function mulAssignSub(a: int, b: int, c: int): int { return a -= b * c }
function mulAssignDiv(a: int, b: int, c: int): int { return a /= b * c }
function mulAssignMod(a: int, b: int, c: int): int { return a %= b * c }
function mulAssignAnd(a: int, b: int, c: int): int { return a &= b * c }
function mulAssignOr(a: int, b: int, c: int): int { return a |= b * c }
function mulAssignXor(a: int, b: int, c: int): int { return a ^= b * c }
function mulAssignShl(a: int, b: int, c: int): int { return a <<= b * c }
function mulAssignShr(a: int, b: int, c: int): int { return a >>= b * c }
function mulAssignUshr(a: int, b: int, c: int): int { return a >>>= b * c }

function main(): int {
    let suite = new arktest.ArkTestsuite("Multiplicative operator precedence")

    suite.addTest("division", () => {
        arktest.assertEQ(mulDiv(8, 3, 2), 4)
    })

    suite.addTest("modulo", () => {
        arktest.assertEQ(mulMod(8, 3, 2), 4)
    })

    suite.addTest("addition", () => {
        arktest.assertEQ(mulAdd(2, 4, 3), 14)
    })

    suite.addTest("subtraction", () => {
        arktest.assertEQ(mulSub(2, 3, 4), -10)
    })

    suite.addTest("cast on result", () => {
        arktest.assertEQ(mulCastRes(2, 255), -2)
    })

    suite.addTest("cast on operand", () => {
        arktest.assertEQ(mulCastOp(255, 3), -3)
    })

    suite.addTest("left shift", () => {
        arktest.assertEQ(mulShl(4, 3, 2), 256)
    })

    suite.addTest("right shift", () => {
        arktest.assertEQ(mulShrNeg(255, 2, 3), -4)
    })

    suite.addTest("unsigned right shift", () => {
        arktest.assertEQ(mulUshrNeg(255, 2, 3), 67108860)
    })

    suite.addTest("relational <", () => {
        arktest.assertFalse(mulLt(255, 8, 8))
    })

    suite.addTest("relational <=", () => {
        arktest.assertFalse(mulLe(255, 2, 8))
    })

    suite.addTest("relational >", () => {
        arktest.assertFalse(mulGt(2, 8, 4))
    })

    suite.addTest("relational >=", () => {
        arktest.assertFalse(mulGe(4, 8, 4))
    })

    suite.addTest("equality ==", () => {
        arktest.assertFalse(mulEqEq(2, 2, 4))
    })

    suite.addTest("equality !=", () => {
        arktest.assertFalse(mulNeq(8, 4, 2))
    })

    suite.addTest("bitwise AND", () => {
        arktest.assertEQ(mulBitAnd(255, 4, 8), 32)
    })

    suite.addTest("bitwise OR", () => {
        arktest.assertEQ(mulBitOr(3, 4, 2), 11)
    })

    suite.addTest("bitwise XOR", () => {
        arktest.assertEQ(mulBitXor(255, 8, 2), 239)
    })

    suite.addTest("ternary in branch", () => {
        arktest.assertEQ(mulTernBranch(3, 4, 0), 4)
    })

    suite.addTest("ternary in condition", () => {
        arktest.assertEQ(mulTernCond(4, 2, 4, 3), 4)
    })

    suite.addTest("assignment +=", () => {
        arktest.assertEQ(mulAssignAdd(2, 4, 3), 14)
    })

    suite.addTest("assignment -=", () => {
        arktest.assertEQ(mulAssignSub(2, 4, 3), -10)
    })

    suite.addTest("assignment /=", () => {
        arktest.assertEQ(mulAssignDiv(8, 2, 3), 1)
    })

    suite.addTest("assignment %=", () => {
        arktest.assertEQ(mulAssignMod(8, 2, 3), 2)
    })

    suite.addTest("assignment &=", () => {
        arktest.assertEQ(mulAssignAnd(255, 2, 3), 6)
    })

    suite.addTest("assignment |=", () => {
        arktest.assertEQ(mulAssignOr(2, 3, 4), 14)
    })

    suite.addTest("assignment ^=", () => {
        arktest.assertEQ(mulAssignXor(255, 4, 8), 223)
    })

    suite.addTest("assignment <<=", () => {
        arktest.assertEQ(mulAssignShl(4, 3, 2), 256)
    })

    suite.addTest("assignment >>=", () => {
        arktest.assertEQ(mulAssignShr(255, 2, 3), 3)
    })

    suite.addTest("assignment >>>=", () => {
        arktest.assertEQ(mulAssignUshr(255, 3, 2), 3)
    })

    return suite.run()
}
