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
desc: Multiplications with Enum and numeric
---*/

enum E { a, b }


function main(): int {

    const suite = new arktest.ArkTestsuite('Test Enum in math expressions');

    let a: number;
    let b: number;
    let c: E = E.b

    suite.addTest('0', () => {
        let res = E.a * E.b;
        arktest.assertEQ(0, res)
    })
    suite.addTest('1', () => {
        let res = b * E.b;
        arktest.assertEQ(0, res)
    })
    suite.addTest('2', () => {
        let res = 1 * E.b;
        arktest.assertEQ(1, res)
    })

    suite.addTest('3', () => {
        let res = E.a * 1;
        arktest.assertEQ(0, res)
    })
    suite.addTest('4', () => {
        let res = E.a * a;
        arktest.assertEQ(0, res)
    })

    suite.addTest('5', () => {
        let res = b * c;
        arktest.assertEQ(0, res)
    })

    suite.addTest('6', () => {
        let res = c * c;
        arktest.assertEQ(1, res)
    })


    return suite.run();
}
