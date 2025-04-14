/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

    let regExpObj10 = new RegExp("(([0-9]+)\.([0-9]+))");
    regExpObj10.exec("abc123.456def");
    testResult = checkTestResult(RegExp.$1, "123.456")
    printTestVerdict(testResult, "test10 $1")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$2, "123")
    printTestVerdict(testResult, "test10 $2")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$3, "456")
    printTestVerdict(testResult, "test10 $3")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$4, "")
    printTestVerdict(testResult, "test10 $4")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$5, "")
    printTestVerdict(testResult, "test10 $5")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$6, "")
    printTestVerdict(testResult, "test10 $6")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$7, "")
    printTestVerdict(testResult, "test10 $7")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$8, "")
    printTestVerdict(testResult, "test10 $8")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$9, "")
    printTestVerdict(testResult, "test10 $9")
    failCounter += testResult

    testResult = checkTestResult(RegExp.input, "abc123.456def")
    printTestVerdict(testResult, "test10 input")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$_get("$_"), "abc123.456def")
    printTestVerdict(testResult, "test10 $_")
    failCounter += testResult

    testResult = checkTestResult(RegExp.lastMatch, "123.456")
    printTestVerdict(testResult, "test10 lastMatch")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$_get("$&"), "123.456")
    printTestVerdict(testResult, "test10 $&")
    failCounter += testResult

    testResult = checkTestResult(RegExp.lastParen, "456")
    printTestVerdict(testResult, "test10 lastParen")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$_get("$+"), "456")
    printTestVerdict(testResult, "test10 $+")
    failCounter += testResult

    testResult = checkTestResult(RegExp.leftContext, "abc")
    printTestVerdict(testResult, "test10 leftContext")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$_get("$`"), "abc")
    printTestVerdict(testResult, "test10 $`")
    failCounter += testResult

    testResult = checkTestResult(RegExp.rightContext, "def")
    printTestVerdict(testResult, "test10 rightContext")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$_get("$'"), "def")
    printTestVerdict(testResult, "test10 $'")
    failCounter += testResult

    regExpObj10.exec("ghi789.012jkl");
    testResult = checkTestResult(RegExp.$1, "789.012")
    printTestVerdict(testResult, "test40 $1")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$2, "789")
    printTestVerdict(testResult, "test40 $2")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$3, "012")
    printTestVerdict(testResult, "test40 $3")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$4, "")
    printTestVerdict(testResult, "test40 $4")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$5, "")
    printTestVerdict(testResult, "test40 $5")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$6, "")
    printTestVerdict(testResult, "test40 $6")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$7, "")
    printTestVerdict(testResult, "test40 $7")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$8, "")
    printTestVerdict(testResult, "test40 $8")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$9, "")
    printTestVerdict(testResult, "test40 $9")
    failCounter += testResult

    testResult = checkTestResult(RegExp.lastMatch, "789.012")
    printTestVerdict(testResult, "test40 lastMatch")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$_get("$&"), "789.012")
    printTestVerdict(testResult, "test40 $&")
    failCounter += testResult

    testResult = checkTestResult(RegExp.input, "ghi789.012jkl")
    printTestVerdict(testResult, "test40 input")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$_get("$_"), "ghi789.012jkl")
    printTestVerdict(testResult, "test40 $_")
    failCounter += testResult

    testResult = checkTestResult(RegExp.lastParen, "012")
    printTestVerdict(testResult, "test40 lastParen")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$_get("$+"), "012")
    printTestVerdict(testResult, "test40 $+")
    failCounter += testResult

    testResult = checkTestResult(RegExp.leftContext, "ghi")
    printTestVerdict(testResult, "test40 leftContext")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$_get("$`"), "ghi")
    printTestVerdict(testResult, "test40 $`")
    failCounter += testResult

    testResult = checkTestResult(RegExp.rightContext, "jkl")
    printTestVerdict(testResult, "test40 rightContext")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$_get("$'"), "jkl")
    printTestVerdict(testResult, "test40 $'")
    failCounter += testResult

    "abc123.456def".replace(regExpObj10, "whocares");
    testResult = checkTestResult(RegExp.$1, "123.456")
    printTestVerdict(testResult, "test20 $1")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$2, "123")
    printTestVerdict(testResult, "test20 $2")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$3, "456")
    printTestVerdict(testResult, "test20 $3")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$4, "")
    printTestVerdict(testResult, "test20 $4")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$5, "")
    printTestVerdict(testResult, "test20 $5")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$6, "")
    printTestVerdict(testResult, "test20 $6")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$7, "")
    printTestVerdict(testResult, "test20 $7")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$8, "")
    printTestVerdict(testResult, "test20 $8")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$9, "")
    printTestVerdict(testResult, "test20 $9")
    failCounter += testResult

    testResult = checkTestResult(RegExp.input, "abc123.456def")
    printTestVerdict(testResult, "test20 input")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$_get("$_"), "abc123.456def")
    printTestVerdict(testResult, "test20 $_")
    failCounter += testResult

    testResult = checkTestResult(RegExp.lastMatch, "123.456")
    printTestVerdict(testResult, "test20 lastMatch")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$_get("$&"), "123.456")
    printTestVerdict(testResult, "test20 $&")
    failCounter += testResult

    testResult = checkTestResult(RegExp.lastParen, "456")
    printTestVerdict(testResult, "test20 lastParen")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$_get("$+"), "456")
    printTestVerdict(testResult, "test20 $+")
    failCounter += testResult

    testResult = checkTestResult(RegExp.leftContext, "abc")
    printTestVerdict(testResult, "test20 leftContext")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$_get("$`"), "abc")
    printTestVerdict(testResult, "test20 $`")
    failCounter += testResult

    testResult = checkTestResult(RegExp.rightContext, "def")
    printTestVerdict(testResult, "test20 rightContext")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$_get("$'"), "def")
    printTestVerdict(testResult, "test20 $'")
    failCounter += testResult

    "ghi789.012jkl".replace(regExpObj10, "whocares");
    testResult = checkTestResult(RegExp.$1, "789.012")
    printTestVerdict(testResult, "test30 $1")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$2, "789")
    printTestVerdict(testResult, "test30 $2")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$3, "012")
    printTestVerdict(testResult, "test30 $3")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$4, "")
    printTestVerdict(testResult, "test30 $4")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$5, "")
    printTestVerdict(testResult, "test30 $5")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$6, "")
    printTestVerdict(testResult, "test30 $6")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$7, "")
    printTestVerdict(testResult, "test30 $7")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$8, "")
    printTestVerdict(testResult, "test30 $8")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$9, "")
    printTestVerdict(testResult, "test30 $9")
    failCounter += testResult

    testResult = checkTestResult(RegExp.lastMatch, "789.012")
    printTestVerdict(testResult, "test30 lastMatch")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$_get("$&"), "789.012")
    printTestVerdict(testResult, "test30 $&")
    failCounter += testResult

    testResult = checkTestResult(RegExp.input, "ghi789.012jkl")
    printTestVerdict(testResult, "test30 input")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$_get("$_"), "ghi789.012jkl")
    printTestVerdict(testResult, "test30 $_")
    failCounter += testResult

    testResult = checkTestResult(RegExp.lastParen, "012")
    printTestVerdict(testResult, "test30 lastParen")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$_get("$+"), "012")
    printTestVerdict(testResult, "test30 $+")
    failCounter += testResult

    testResult = checkTestResult(RegExp.leftContext, "ghi")
    printTestVerdict(testResult, "test30 leftContext")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$_get("$`"), "ghi")
    printTestVerdict(testResult, "test30 $`")
    failCounter += testResult

    testResult = checkTestResult(RegExp.rightContext, "jkl")
    printTestVerdict(testResult, "test30 rightContext")
    failCounter += testResult

    testResult = checkTestResult(RegExp.$_get("$'"), "jkl")
    printTestVerdict(testResult, "test30 $'")
    failCounter += testResult

    let regExpObj40 = new RegExp("^([a-z]+): (.*)")
    regExpObj40.exec("regexp: /foobar/\nldkfj sdlkfj sdkl")
    testResult = checkTestResult(RegExp.$1, "regexp")
    printTestVerdict(testResult, "test50 $1")
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

