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

const success = 0;
const fail = 1;

function main(): int {
    let failCounter: int = 0;
    let testResult: int = 0;

    let regExpObj10 = new RegExp("Brawn Fox");
    testResult = checkTestResult(regExpObj10, "Brawn Fox", "", false, false, false, false, false, false, false, false)
    printTestVerdict(testResult, "test10")
    failCounter += testResult

    let regExpObj20 = new RegExp("");
    testResult = checkTestResult(regExpObj20, "(?:)", "", false, false, false, false, false, false, false, false)
    printTestVerdict(testResult, "test20")
    failCounter += testResult

    let regExpObj30 = new RegExp("Brawn", "gsu");
    testResult = checkTestResult(regExpObj30, "Brawn", "gsu", true, true, false, false, false, false, true, false)
    printTestVerdict(testResult, "test30")
    failCounter += testResult

    let regExpObj40 = new RegExp("Brawn", "g");
    testResult = checkTestResult(regExpObj40, "Brawn", "g", true, false, false, false, false, false, false, false)
    printTestVerdict(testResult, "test40")
    failCounter += testResult

    let regExpObj50 = new RegExp("Brawn", "s");
    testResult = checkTestResult(regExpObj50, "Brawn", "s", false, true, false, false, false, false, false, false)
    printTestVerdict(testResult, "test50")
    failCounter += testResult

    let regExpObj60 = new RegExp("Brawn", "u");
    testResult = checkTestResult(regExpObj60, "Brawn", "u", false, false, false, false, false, false, true, false)
    printTestVerdict(testResult, "test60")
    failCounter += testResult

    let regExpObj70 = new RegExp("Brawn", "gu");
    testResult = checkTestResult(regExpObj70, "Brawn", "gu", true, false, false, false, false, false, true, false)
    printTestVerdict(testResult, "test70")
    failCounter += testResult

    let regExpObj80 = new RegExp("Brawn", "gs");
    testResult = checkTestResult(regExpObj80, "Brawn", "gs", true, true, false, false, false, false, false, false)
    printTestVerdict(testResult, "test80")
    failCounter += testResult

    let regExpObj90 = new RegExp("Brawn", "su");
    testResult = checkTestResult(regExpObj90, "Brawn", "su", false, true, false, false, false, false, true, false)
    printTestVerdict(testResult, "test90")
    failCounter += testResult

    let regExpObj100 = new RegExp("", "dgimsuy");
    testResult = checkTestResult(regExpObj100, "(?:)", "dgimsuy", true, true, true, true, true, true, true, false)
    printTestVerdict(testResult, "test100")
    failCounter += testResult

    let regExpObj110 = new RegExp("", "yusmigd");
    testResult = checkTestResult(regExpObj110, "(?:)", "dgimsuy", true, true, true, true, true, true, true, false)
    printTestVerdict(testResult, "test110")
    failCounter += testResult

    let regExpObj230 = new RegExp("(?:\u{23456})");
    testResult = checkTestResult(regExpObj230, "(?:𣑖)", "", false, false, false, false, false, false, false, false)
    printTestVerdict(testResult, "test230")
    failCounter += testResult

    failCounter += test(testRegExpParse(),"Regpex parse failed and returned SyntaxError");
    failCounter += test(testRegExpParseUnclosedGroup(),"Regpex parse failed and returned SyntaxError");
    failCounter += test(testRegExpParseQuantifierRange(),"Regpex parse failed and returned SyntaxError");
    failCounter += test(testRegExpParseNestedQuantifiers(),"Regpex parse failed and returned SyntaxError");

    if (failCounter > 0) return 1
    return 0;
}

function checkTestResult(actual : RegExp, pattern: String, flags: String,
    global: boolean, dotAll: boolean, hasIndices: boolean,
    ignoreCase: boolean, multiline: boolean, sticky: boolean,
    unicode: boolean, unicodeSets: boolean): int {
    let checkResult : boolean = true;
    if (!actual.source.equals(pattern)) {
        console.println("Source " + actual.source + " is not equal expected " + pattern);
        checkResult = false;
    }
    if (!actual.flags.equals(flags)) {
        console.println("Flags " + actual.flags + " is not equal expected " + flags);
        checkResult = false;
    }

    if (actual.global != global) {
        console.println("Global " + actual.global + " is not equal expected " + global);
        checkResult = false;
    }
    if (actual.unicode != unicode) {
        console.println("Unicode " + actual.unicode + " is not equal expected " + unicode);
        checkResult = false;
    }
    if (actual.dotAll != dotAll) {
        console.println("dotAll " + actual.dotAll + " is not equal expected " + dotAll);
        checkResult = false;
    }
    if (actual.hasIndices != hasIndices) {
        console.println("hasIndices " + actual.hasIndices + " is not equal expected " + hasIndices);
        checkResult = false;
    }
    if (actual.multiline != multiline) {
        console.println("multiline " + actual.multiline + " is not equal expected " + multiline);
        checkResult = false;
    }
    if (actual.sticky != sticky) {
        console.println("sticky " + actual.sticky + " is not equal expected " + sticky);
        checkResult = false;
    }
    if (actual.unicodeSets != unicodeSets) {
        console.println("unicodeSets " + actual.unicodeSets + " is not equal expected " + unicodeSets);
        checkResult = false;
    }

    if (!checkResult) return 1;
    return 0;
}

function printTestVerdict(flag: int, testName: String) {
    if (flag == 0) {
        console.println(testName + " : PASSED")
    } else {
        console.println(testName + " : FAILED")
    }
}

function testRegExpParse(): int {
    try {
        let regExpObj120 = new RegExp("a**");
    } catch (err) {
        if (err instanceof SyntaxError) {
            return success
        }
        return fail
    }
    return fail
}

function testRegExpParseUnclosedGroup(): int {
    try {
        let regExpObj120 = new RegExp("(abc|def");
    } catch (err) {
        if (err instanceof SyntaxError) {
            return success
        }
        return fail
    }
    return fail
}

function testRegExpParseQuantifierRange(): int {
    try {
        let regExpObj120 = new RegExp("a{3,2}");
    } catch (err) {
        if (err instanceof SyntaxError) {
            return success
        }
        return fail
    }
    return fail
}

function testRegExpParseNestedQuantifiers(): int {
    try {
        let regExpObj120 = new RegExp("a++b");
    } catch (err) {
        if (err instanceof SyntaxError) {
            return success
        }
        return fail
    }
    return fail
}

function test(result: int, message: String): int {
    if (result == 0) {
      return success;
    }
    console.println("FAILED: " + message);
    return fail;
}
