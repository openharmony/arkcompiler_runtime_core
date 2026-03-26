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
desc: exponentiation operator with other operators
---*/

class IntKeepper {
    x: int = 2;
    y: int = 3;
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Exponentiation operator precedence")

    suite.addTest("grouping", () => {
        let x: number = (1 + 2)**(2 + 1);
        arktest.assertEQ(x, 27)
    })

    suite.addTest("member access", () => {
        let v = new IntKeepper();
        let x: number = v.x ** v.y;
        arktest.assertEQ(x, 8)
    })

    suite.addTest("index", () => {
        let a: int[] = [2, 3];
        let x: number = a[1] ** a[0];
        arktest.assertEQ(x, 9)
    })

    suite.addTest("prefix increment", () => {
        let x: number = 1;
        x = ++x ** ++x;
        arktest.assertEQ(x, 8)
    })

    suite.addTest("postfix decrement", () => {
        let x: number = 3;
        x = x-- ** x--;
        arktest.assertEQ(x, 9)
    })

    suite.addTest("unary minus", () => {
        let x: number = -3;
        x = x ** -x;
        arktest.assertEQ(x, -27)
    })

    suite.addTest("associativity", () => {
        let x: number = 4 ** 3 ** 2;
        arktest.assertEQ(x, 262144)
    })

    suite.addTest("multiplication precedence", () => {
        let x: number = 2*3**2;
        arktest.assertEQ(x, 18)
    })

    suite.addTest("addition and subtraction precedence", () => {
        let x: number = 11+3**2-19;
        arktest.assertEQ(x, 1)
    })

    return suite.run()
}
