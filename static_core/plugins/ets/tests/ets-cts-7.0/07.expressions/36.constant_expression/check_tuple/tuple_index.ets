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
    An index expression operand for tuples must be a constant expression of an integer type.
---*/

let tup: [number, string] = [42.0, 'abc']

function main() {
    const c1 = -1

    let tests = new arktest.ArkTestsuite('Tuple index is a constant expression')

    tests.addTest('Unary operations', () => {
        arktest.assertEQ(tup[+1], 'abc')
        arktest.assertEQ(tup[-c1], 'abc')
        arktest.assertEQ(tup[~c1], 42.0)
    })

    tests.addTest('Additive operations', () => {
        arktest.assertEQ(tup[1 + c1 + 1], 'abc')
        arktest.assertEQ(tup[c1 + c1 + 3], 'abc')
        arktest.assertEQ(tup[-1 - c1 - c1], 'abc')
    })

    tests.addTest('Multiplicative operations', () => {
        arktest.assertEQ(tup[c1 * c1], 'abc')
        arktest.assertEQ(tup[c1 / 2], 42.0)
        arktest.assertEQ(tup[43 % 2], 'abc')
    })

    tests.addTest('Shift operations', () => {
        arktest.assertEQ(tup[2 >> 2], 42.0)
        arktest.assertEQ(tup[2 >>> 1], 'abc')
        arktest.assertEQ(tup[1 << 1 - 1], 'abc')
    })

    tests.addTest('Bitwise and logical operations', () => {
        const c0 = 0
        arktest.assertEQ(tup[c0 | 1], 'abc')
        arktest.assertEQ(tup[c0 | 1 & c0], 42.0)
        arktest.assertEQ(tup[1 ^ 2 & 1 ], 'abc')
    })

    tests.addTest('Ternary and relational', () => {
        arktest.assertEQ(tup[c1 + 1 == 0 ? c1 + 2 : 0], 'abc')
        arktest.assertEQ(tup[c1 > 0 ? 42 : 1], 'abc')
    })

    tests.addTest('Cast expression for numeric literal', () => {
        arktest.assertEQ(tup[0 as int], 42.0)
        arktest.assertEQ(tup[1 as short], 'abc')
    })

    return tests.run()
}