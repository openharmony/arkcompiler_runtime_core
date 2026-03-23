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
desc: conditional && and || operators on boolean values
---*/

let res = "";

function t1(): boolean { res += "1"; return true; }
function t2(): boolean { res += "2"; return false; }
function t3(): boolean { res += "3"; return true; }
function t4(): boolean { res += "4"; return false; }

function main(): int {
    let suite = new arktest.ArkTestsuite("Conditional && and || operators on boolean values")

    suite.addTest("AND then OR without NOT - TFT", () => {
        res = ""
        arktest.assertTrue(t1() && t2() || t3())  // TFT
        arktest.assertEQ(res, "123")
    })

    suite.addTest("AND then OR without NOT - short circuit", () => {
        res = ""
        arktest.assertTrue(t1() || t2() && t3())  // TFT
        arktest.assertEQ(res, "1")
    })

    suite.addTest("OR then AND without NOT - FTF", () => {
        res = ""
        arktest.assertFalse(t2() || t3() && t4())  // FTF
        arktest.assertEQ(res, "234")
    })

    suite.addTest("OR then AND without NOT - short circuit", () => {
        res = ""
        arktest.assertFalse(t2() && t3() || t4())  // FTF
        arktest.assertEQ(res, "24")
    })

    suite.addTest("NOT on first operand - FFT", () => {
        res = ""
        arktest.assertTrue(!t1() && t2() || t3())  // FFT
        arktest.assertEQ(res, "13")
    })

    suite.addTest("NOT on first operand - short circuit", () => {
        res = ""
        arktest.assertFalse(!t1() || t2() && t3())  // FFT
        arktest.assertEQ(res, "12")
    })

    suite.addTest("NOT on first with true result - TTF", () => {
        res = ""
        arktest.assertTrue(!t2() || t3() && t4())  // TTF
        arktest.assertEQ(res, "2")
    })

    suite.addTest("NOT on first with true result - short circuit", () => {
        res = ""
        arktest.assertTrue(!t2() && t3() || t4())  // TTF
        arktest.assertEQ(res, "23")
    })

    suite.addTest("NOT on middle operand - TTT", () => {
        res = ""
        arktest.assertTrue(t1() && !t2() || t3())  // TTT
        arktest.assertEQ(res, "12")
    })

    suite.addTest("NOT on middle operand - short circuit", () => {
        res = ""
        arktest.assertTrue(t1() || !t2() && t3())  // TTT
        arktest.assertEQ(res, "1")
    })

    suite.addTest("NOT on middle with false result - FFF", () => {
        res = ""
        arktest.assertFalse(t2() || !t3() && t4())  // FFF
        arktest.assertEQ(res, "23")
    })

    suite.addTest("NOT on middle with false result - short circuit", () => {
        res = ""
        arktest.assertFalse(t2() && !t3() || t4())  // FFF
        arktest.assertEQ(res, "24")
    })

    return suite.run()
}
