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
desc: remainder operator with other operators
---*/

function modMul(a: int, b: int, c: int): int { return a * b % c }
function modDiv(a: int, b: int, c: int): int { return a / b % c }
function modAdd(a: int, b: int, c: int): int { return a + b % c }
function modSub(a: int, b: int, c: int): int { return a - b % c }
function modCastRes(a: int, b: int): int { return (a % b).toByte() }
function modCastOp(a: int, b: int): int { return a.toByte() % b }
function modShl(a: int, b: int, c: int): int { return a << b % c }
function modShr(a: int, b: int, c: int): int { return a >> b % c }
function modUshr(a: int, b: int, c: int): int { return -a >>> b % c }
function modLt(a: int, b: int, c: int): boolean { return a < b % c }
function modLe(a: int, b: int, c: int): boolean { return a <= b % c }
function modGt(a: int, b: int, c: int): boolean { return a > b % c }
function modGe(a: int, b: int, c: int): boolean { return a >= b % c }
function modEqEq(a: int, b: int, c: int): boolean { return a == b % c }
function modNeq(a: int, b: int, c: int): boolean { return a != b % c }
function modBitAnd(a: int, b: int, c: int): int { return a & b % c }
function modBitOr(a: int, b: int, c: int): int { return a | b % c }
function modBitXor(a: int, b: int, c: int): int { return a ^ b % c }
function modTernBranch(a: int, b: int, c: int): int { return a > b ? c : a % c }
function modTernCond(a: int, b: int, c: int, d: int): int { return a % b > c ? c : d }
function modAssignAdd(a: int, b: int, c: int): int { return a += b % c }
function modAssignSub(a: int, b: int, c: int): int { return a -= b % c }
function modAssignMul(a: int, b: int, c: int): int { return a *= b % c }
function modAssignDiv(a: int, b: int, c: int): int { return a /= b % c }
function modAssignAnd(a: int, b: int, c: int): int { return a &= b % c }
function modAssignOr(a: int, b: int, c: int): int { return a |= b % c }
function modAssignXor(a: int, b: int, c: int): int { return a ^= b % c }
function modAssignShl(a: int, b: int, c: int): int { return a <<= b % c }
function modAssignShr(a: int, b: int, c: int): int { return a >>= b % c }
function modAssignUshr(a: int, b: int, c: int): int { return a >>>= b % c }

function main(): int {
    let suite = new arktest.ArkTestsuite("Remainder operator precedence")

    suite.addTest("multiplication", () => {
        arktest.assertEQ(modMul(3, 7, 3), 0)
    })

    suite.addTest("division", () => {
        arktest.assertEQ(modDiv(8, 3, 2), 0)
    })

    suite.addTest("addition", () => {
        arktest.assertEQ(modAdd(2, 7, 2), 3)
    })

    suite.addTest("subtraction", () => {
        arktest.assertEQ(modSub(8, 7, 4), 5)
    })

    suite.addTest("cast on result", () => {
        arktest.assertEQ(modCastRes(518, 255), 8)
    })

    suite.addTest("cast on operand", () => {
        arktest.assertEQ(modCastOp(518, 255), 6)
    })

    suite.addTest("left shift", () => {
        arktest.assertEQ(modShl(4, 3, 2), 8)
    })

    suite.addTest("right shift", () => {
        arktest.assertEQ(modShr(255, 3, 2), 127)
    })

    suite.addTest("unsigned right shift", () => {
        arktest.assertEQ(modUshr(255, 7, 4), 536870880)
    })

    suite.addTest("relational <", () => {
        arktest.assertFalse(modLt(8, 518, 255))
    })

    suite.addTest("relational <=", () => {
        arktest.assertFalse(modLe(255, 518, 255))
    })

    suite.addTest("relational >", () => {
        arktest.assertFalse(modGt(3, 7, 4))
    })

    suite.addTest("relational >=", () => {
        arktest.assertFalse(modGe(2, 7, 4))
    })

    suite.addTest("equality ==", () => {
        arktest.assertFalse(modEqEq(4, 8, 3))
    })

    suite.addTest("equality !=", () => {
        arktest.assertFalse(modNeq(2, 8, 3))
    })

    suite.addTest("bitwise AND", () => {
        arktest.assertEQ(modBitAnd(7, 255, 7), 3)
    })

    suite.addTest("bitwise OR", () => {
        arktest.assertEQ(modBitOr(8, 3, 2), 9)
    })

    suite.addTest("bitwise XOR", () => {
        arktest.assertEQ(modBitXor(255, 7, 2), 254)
    })

    suite.addTest("ternary in branch", () => {
        arktest.assertEQ(modTernBranch(8, 2, 4), 4)
    })

    suite.addTest("ternary in condition", () => {
        arktest.assertEQ(modTernCond(7, 4, 2, 8), 2)
    })

    suite.addTest("assignment +=", () => {
        arktest.assertEQ(modAssignAdd(2, 7, 3), 3)
    })

    suite.addTest("assignment -=", () => {
        arktest.assertEQ(modAssignSub(2, 7, 3), 1)
    })

    suite.addTest("assignment *=", () => {
        arktest.assertEQ(modAssignMul(8, 7, 3), 8)
    })

    suite.addTest("assignment /=", () => {
        arktest.assertEQ(modAssignDiv(8, 7, 2), 8)
    })

    suite.addTest("assignment &=", () => {
        arktest.assertEQ(modAssignAnd(3, 7, 3), 1)
    })

    suite.addTest("assignment |=", () => {
        arktest.assertEQ(modAssignOr(2, 7, 3), 3)
    })

    suite.addTest("assignment ^=", () => {
        arktest.assertEQ(modAssignXor(255, 7, 4), 252)
    })

    suite.addTest("assignment <<=", () => {
        arktest.assertEQ(modAssignShl(4, 3, 2), 8)
    })

    suite.addTest("assignment >>=", () => {
        arktest.assertEQ(modAssignShr(255, 3, 2), 127)
    })

    suite.addTest("assignment >>>=", () => {
        arktest.assertEQ(modAssignUshr(255, 3, 2), 127)
    })

    return suite.run()
}
