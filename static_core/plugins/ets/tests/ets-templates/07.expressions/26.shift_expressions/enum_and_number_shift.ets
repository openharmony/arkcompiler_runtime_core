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
desc: Shift operations with Enum and numeric
---*/

enum E { a, b }


function main(): int {

    const suite = new arktest.ArkTestsuite('Test Enum in math expressions');

    let a: number;
    let b: number;
    let c: E = E.b

    // operator <<
    suite.addTest('0', () => {
        let res = c << a;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(1, res)
    })

    suite.addTest('1', () => {
        let res = c << c;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(2, res)
    })
    suite.addTest('2', () => {
        let res = a << c;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(0, res)
    })

    suite.addTest('3', () => {
        let res = E.a << a;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(0, res)
    })

    suite.addTest('4', () => {
        let res = E.a << E.b;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(0, res)
    })
    suite.addTest('5', () => {
        let res = E.a << 1;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(0, res)
    })
    suite.addTest('6', () => {
        let res = a << E.b;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(0, res)
    })

    suite.addTest('7', () => {
        let res = 1 << E.b;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(2, res)
    })

    // operator >>
    suite.addTest('8', () => {
        let res = c >> a;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(1, res)
    })

    suite.addTest('9', () => {
        let res = c >> c;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(0, res)
    })
    suite.addTest('10', () => {
        let res = a >> c;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(0, res)
    })

    suite.addTest('11', () => {
        let res = E.a >> a;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(0, res)
    })

    suite.addTest('12', () => {
        let res = E.a >> E.b;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(0, res)
    })
    suite.addTest('13', () => {
        let res = E.a >> 1;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(0, res)
    })
    suite.addTest('14', () => {
        let res = a >> E.b;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(0, res)
    })

    suite.addTest('15', () => {
        let res = 1 >> E.b;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(0, res)
    })

    // operator >>>
    suite.addTest('16', () => {
        let res = c >>> a;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(1, res)
    })

    suite.addTest('17', () => {
        let res = c >>> c;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(0, res)
    })
    suite.addTest('18', () => {
        let res = a >>> c;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(0, res)
    })

    suite.addTest('19', () => {
        let res = E.a >>> a;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(0, res)
    })

    suite.addTest('20', () => {
        let res = E.a >>> E.b;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(0, res)
    })
    suite.addTest('21', () => {
        let res = E.a >>> 1;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(0, res)
    })
    suite.addTest('22', () => {
        let res = a >>> E.b;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(0, res)
    })

    suite.addTest('23', () => {
        let res = 1 >>> E.b;
        arktest.assertTrue(res instanceof Int)
        arktest.assertEQ(0, res)
    })

    return suite.run();
}
