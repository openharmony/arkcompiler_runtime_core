/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

function main(): int {
    const suite = new ArkTestsuite("RegExpExec Tests");
    suite.addTest("test RegExp.exce1", testRegExpExec1);
    suite.addTest("test RegExp.exce2", testRegExpExec2);
    suite.addTest("test RegExp.exce3", testRegExpExec3);
    suite.addTest("test RegExp.exce4", testRegExpExec4);
    suite.addTest("test RegExp.exce5", testRegExpExec5);
    suite.addTest("test RegExp.exce6", testRegExpExec6);
    suite.addTest("test RegExp.exce7", testRegExpExec7);

    return suite.run();
}

function testRegExpExec1(): void {
    let arr = new RegExp("\x00").exec("\u0000");
    assertNE(arr, null);
    if (arr != null) {
        assertEQ(arr[0].codePointAt(0), 0, "testRegExpExec1");
    }
}

function testRegExpExec2(): void {
    let arr = new RegExp("ab\x00g").exec("ab\u0000grfd");
    assertNE(arr, null);
    if (arr != null) {
        assertEQ(arr[0].codePointAt(0), 97, "testRegExpExec2");
        assertEQ(arr[0].codePointAt(1), 98, "testRegExpExec2");
        assertEQ(arr[0].codePointAt(2), 0, "testRegExpExec2");
        assertEQ(arr[0].codePointAt(3), 103, "testRegExpExec2");
    }
}

function testRegExpExec3(): void {
    let arr = new RegExp("ab").exec("ab\u0000grfd");
    assertNE(arr, null);
    if (arr != null) {
        assertEQ(arr[0].codePointAt(0), 97, "testRegExpExec3");
        assertEQ(arr[0].codePointAt(1), 98, "testRegExpExec3");
    }
}

function testRegExpExec4(): void {
    let arr = new RegExp("ab\x00g").exec("abgrfddfs");
    assertEQ(arr, null, "testRegExpExec4");
}

function testRegExpExec5(): void {
    let arr = new RegExp("\x00").exec("");
    assertEQ(arr, null, "testRegExpExec5");
}

function testRegExpExec6(): void {
    let arr = new RegExp("").exec("\u0000");
    assertNE(arr, null);
    if (arr != null) {
        assertEQ(arr[0].codePointAt(0), undefined, "testRegExpExec6");
    }
}

function testRegExpExec7(): void {
    let arr = new RegExp("").exec("");
    assertNE(arr, null);
    if (arr != null) {
        assertEQ(arr[0].codePointAt(0), undefined, "testRegExpExec7");
    }
}