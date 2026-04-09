/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
    The execution of a binary operator that is an integer division ‘/’,
    or integer remainder ‘%’
    can throw ArithmeticError only after the evaluations of both operands complete normally.
---*/

const suite = new arktest.ArkTestsuite("ArithmeticError order of evaluation")

suite.addTest("Division: lhs RangeError takes precedence over rhs division by zero", () => {
    let a = new int[2](0);
    let x: int = 37;
    try {
        x = a[2] / 0;
    } catch (e) {
        arktest.assertTrue(e instanceof RangeError);
    }
    arktest.assertEQ(x, 37);
})

suite.addTest("Remainder: lhs RangeError takes precedence over rhs remainder by zero", () => {
    let a = new int[2](0);
    let x: int = 37;
    try {
        x = a[2] % 0;
    } catch (e) {
        arktest.assertTrue(e instanceof RangeError);
    }
    arktest.assertEQ(x, 37);
})

suite.addTest("Remainder by zero throws ArithmeticError after both operands evaluated", () => {
    let a = new int[2](0);
    let x: int = 37;
    try {
        x = a[0] % a[1];
    } catch (e) {
        arktest.assertTrue(e instanceof ArithmeticError);
    }
    arktest.assertEQ(x, 37);
})

function main() {
    suite.run()
}
