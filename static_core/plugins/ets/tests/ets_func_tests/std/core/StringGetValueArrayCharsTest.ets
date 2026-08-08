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

/**
 * Test cases for String.getValueArrayChars() method
 */

// Test: empty string with no arguments
function emptyStringNoArgs(): void {
    const str: string = ''
    const result = str.getValueArrayChars()
    arktest.assertEQ(result.length, 0)
}

// Test: empty string with range
function emptyStringWithRange(): void {
    const str: string = ''
    const result = str.getValueArrayChars(0, 0)
    arktest.assertEQ(result.length, 0)
}

// Test: single character string - no args
function singleCharNoArgs(): void {
    const str: string = 'A'
    const result = str.getValueArrayChars()
    arktest.assertEQ(result.length, 1)
    arktest.assertEQ(result[0], c'A')
}

// Test: single character string - with range
function singleCharWithRange(): void {
    const str: string = 'A'
    const result = str.getValueArrayChars(0, 1)
    arktest.assertEQ(result.length, 1)
    arktest.assertEQ(result[0], c'A')
}

// Test: multi-character string - no args
function multiCharNoArgs(): void {
    const str: string = 'abcd'
    const result = str.getValueArrayChars()
    arktest.assertEQ(result.length, 4)
    arktest.assertEQ(result[0], c'a')
    arktest.assertEQ(result[1], c'b')
    arktest.assertEQ(result[2], c'c')
    arktest.assertEQ(result[3], c'd')
}

// Test: multi-character string - with partial range
function multiCharPartialRange(): void {
    const str: string = 'abcd'
    const result = str.getValueArrayChars(1, 3)
    arktest.assertEQ(result.length, 2)
    arktest.assertEQ(result[0], c'b')
    arktest.assertEQ(result[1], c'c')
}

// Test: multi-character string - from beginning
function multiCharFromBeginning(): void {
    const str: string = 'abcd'
    const result = str.getValueArrayChars(0, 2)
    arktest.assertEQ(result.length, 2)
    arktest.assertEQ(result[0], c'a')
    arktest.assertEQ(result[1], c'b')
}

// Test: multi-character string - to end
function multiCharToEnd(): void {
    const str: string = 'abcd'
    const result = str.getValueArrayChars(2, 4)
    arktest.assertEQ(result.length, 2)
    arktest.assertEQ(result[0], c'c')
    arktest.assertEQ(result[1], c'd')
}

// Test: string with unicode characters
function unicodeChars(): void {
    const str: string = '中文😀ABC'
    const result = str.getValueArrayChars()
    arktest.assertEQ(result.length, 7)
    arktest.assertEQ(result[0], c'中')
    arktest.assertEQ(result[1], c'文')
    // Surrogate pair for 😀
    arktest.assertEQ(result[2].toInt(), 0xD83D)
    arktest.assertEQ(result[3].toInt(), 0xDE00)
    arktest.assertEQ(result[4], c'A')
    arktest.assertEQ(result[5], c'B')
    arktest.assertEQ(result[6], c'C')
}

// Test: string with special characters
function specialChars(): void {
    const str: string = '\t\n\r'
    const result = str.getValueArrayChars()
    arktest.assertEQ(result.length, 3)
    arktest.assertEQ(result[0], c'\t')
    arktest.assertEQ(result[1], c'\n')
    arktest.assertEQ(result[2], c'\r')
}

// Test: result equals getChars() output
function equalsGetChars(): void {
    const str: string = 'hello'
    const vaResult = str.getValueArrayChars()
    const arrResult = str.getChars()
    arktest.assertEQ(vaResult.length, arrResult.length)
    for (let i: int = 0; i < vaResult.length; i++) {
        arktest.assertEQ(vaResult[i], arrResult[i])
    }
}

// Test: large string
function largeString(): void {
    const str: string = 'a'.repeat(1000)
    const result = str.getValueArrayChars()
    arktest.assertEQ(result.length, 1000)
    arktest.assertEQ(result[0], c'a')
    arktest.assertEQ(result[999], c'a')
}

// Test: zero-length range
function zeroLengthRange(): void {
    const str: string = 'abcd'
    const result = str.getValueArrayChars(2, 2)
    arktest.assertEQ(result.length, 0)
}

// Test: modify returned ValueArray
function modifyReturnedArray(): void {
    const str: string = 'abcd'
    const result = str.getValueArrayChars()
    result[0] = c'X'
    // Original string should not be affected
    arktest.assertEQ(str[0], 'a')
}

// Test: use ValueArray to create new String
function valueArrayToString(): void {
    const str: string = 'abcd'
    const va = str.getValueArrayChars()
    const newStr = new String(va)
    arktest.assertEQ(newStr, 'abcd')
}

function main(): int {
    const suite = new arktest.ArkTestsuite('std.core.String.getValueArrayChars method tests')

    suite.addTest('empty string - no args', emptyStringNoArgs)
    suite.addTest('empty string - with range', emptyStringWithRange)
    suite.addTest('single char - no args', singleCharNoArgs)
    suite.addTest('single char - with range', singleCharWithRange)
    suite.addTest('multi char - no args', multiCharNoArgs)
    suite.addTest('multi char - partial range', multiCharPartialRange)
    suite.addTest('multi char - from beginning', multiCharFromBeginning)
    suite.addTest('multi char - to end', multiCharToEnd)
    suite.addTest('unicode characters', unicodeChars)
    suite.addTest('special characters', specialChars)
    suite.addTest('equals getChars output', equalsGetChars)
    suite.addTest('large string', largeString)
    suite.addTest('zero length range', zeroLengthRange)
    suite.addTest('modify returned array', modifyReturnedArray)
    suite.addTest('ValueArray to String', valueArrayToString)

    return suite.run()
}
