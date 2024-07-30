/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
    let failCounter: int = 0;
    let testResult: int = 0;

    let regExpObj10 = new RegExp("foo");
    regExpObj10.exec("table football");
    testResult = checkTestResult(RegExp.lastMatch, "foo")
    printTestVerdict(testResult, "test10")
    failCounter += testResult

    let regExpObj20 = new RegExp("table");
    regExpObj20.exec("table football");
    testResult = checkTestResult(RegExp.lastMatch, "table")
    printTestVerdict(testResult, "test20")
    failCounter += testResult

    let regExpObj30 = new RegExp("ball");
    regExpObj30.exec("table football");
    testResult = checkTestResult(RegExp.lastMatch, "ball")
    printTestVerdict(testResult, "test30")
    failCounter += testResult

    let regExpObj40 = new RegExp("foot");
    regExpObj40.exec("table football");
    testResult = checkTestResult(RegExp.lastMatch, "foot")
    printTestVerdict(testResult, "test40")
    failCounter += testResult

    let regExpObj50 = new RegExp("a*");
    regExpObj50.exec("aabaac");
    testResult = checkTestResult(RegExp.lastMatch, "aa")
    printTestVerdict(testResult, "test50")
    failCounter += testResult

    let regExpObj60 = new RegExp("a?");
    regExpObj60.exec("ab");
    testResult = checkTestResult(RegExp.lastMatch, "a")
    printTestVerdict(testResult, "test60")
    failCounter += testResult

    let regExpObj70 = new RegExp("a?");
    regExpObj70.exec("b");
    testResult = checkTestResult(RegExp.lastMatch, "")
    printTestVerdict(testResult, "test70")
    failCounter += testResult

    let regExpObj80 = new RegExp("a*");
    regExpObj80.exec("a");
    testResult = checkTestResult(RegExp.lastMatch, "a")
    printTestVerdict(testResult, "test80")
    failCounter += testResult

    let regExpObj90 = new RegExp("abc");
    regExpObj90.exec("ABC");
    testResult = checkTestResult(RegExp.lastMatch, "")
    printTestVerdict(testResult, "test90")
    failCounter += testResult

    if (failCounter > 0) return 1
    return 0;
}

function checkTestResult(actual: String, expected: String): int {
    if (actual == expected) {
        return 0;
    } else {
        console.println("Actual: " + actual + ", Expected: " + expected)
        return 1;
    }
}

function printTestVerdict(flag: int, testName: String) {
    if (flag == 0) {
        console.println(testName + " : PASSED")
    } else {
        console.println(testName + " : FAILED")
    }
}

