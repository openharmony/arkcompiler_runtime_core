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

import { testStringCasesInstance } from 'testDecl';
import { testStringMethodsInstance } from 'testDecl';

function testHugeString(): void {
    let hugeStr = testStringCasesInstance.hugeString;
    arktest.assertEQ(hugeStr.length, 10000000);
    arktest.assertEQ(hugeStr.charAt(0).toString(), 'a');
    arktest.assertEQ(hugeStr.charAt(9999999).toString(), 'a');
    arktest.assertEQ(hugeStr.substring(0, 5), 'aaaaa');
}

function testChineseString(): void {
    let chineseStr = testStringCasesInstance.chineseString;
    arktest.assertEQ(chineseStr.length > 0, true);
    arktest.assertEQ(chineseStr.includes('你好'), true);
    arktest.assertEQ(chineseStr.includes('繁體字'), true);
    arktest.assertEQ(chineseStr.includes('한국어'), true);
    arktest.assertEQ(chineseStr.includes('日本語'), true);
}

function testEmojiString(): void {
    let emojiStr = testStringCasesInstance.emojiString;
    arktest.assertEQ(emojiStr.length > 0, true);
    arktest.assertEQ(emojiStr.includes('😀'), true);
    arktest.assertEQ(emojiStr.includes('🌍'), false);
    arktest.assertEQ(emojiStr.includes('🎉'), false);
    arktest.assertEQ(emojiStr.includes('🚀'), false);
}

function testSpecialCharsString(): void {
    let specialStr = testStringCasesInstance.specialCharsString;
    arktest.assertEQ(specialStr.length > 0, true);
    arktest.assertEQ(specialStr.includes('!'), true);
    arktest.assertEQ(specialStr.includes('@'), true);
    arktest.assertEQ(specialStr.includes('#'), true);
    arktest.assertEQ(specialStr.includes('$'), true);
    arktest.assertEQ(specialStr.includes('%'), true);
}

function testMixedString(): void {
    let mixedStr = testStringCasesInstance.mixedString;
    arktest.assertEQ(mixedStr.length > 0, true);
    arktest.assertEQ(mixedStr.includes('Hello'), true);
    arktest.assertEQ(mixedStr.includes('世界'), true);
    arktest.assertEQ(mixedStr.includes('🌍'), true);
    arktest.assertEQ(mixedStr.includes('你好'), true);
    arktest.assertEQ(mixedStr.includes('繁體'), true);
    arktest.assertEQ(mixedStr.includes('한국어'), true);
    arktest.assertEQ(mixedStr.includes('😊'), true);
}

function testEmptyString(): void {
    let emptyStr = testStringCasesInstance.emptyString;
    arktest.assertEQ(emptyStr.length, 0);
    arktest.assertEQ(emptyStr === '', true);
}

function testWhitespaceString(): void {
    let whitespaceStr = testStringCasesInstance.whitespaceString;
    arktest.assertEQ(whitespaceStr.length > 0, true);
    let trimmed = whitespaceStr.trim();
    arktest.assertEQ(trimmed.length, 0);
}

function testStringMethodsSubstring(): void {
    let result = testStringMethodsInstance.substring(0, 5);
    arktest.assertEQ(result, 'Hello');
}

function testStringMethodsCharAt(): void {
    let result = testStringMethodsInstance.charAt(0);
    arktest.assertEQ(result, 'H');
}

function testStringMethodsIndexOf(): void {
    let result1 = testStringMethodsInstance.indexOf('World');
    arktest.assertEQ(result1, 6);
    let result2 = testStringMethodsInstance.indexOf('你好');
    arktest.assertEQ(result2 > 0, true);
}

function testStringMethodsIncludes(): void {
    let result1 = testStringMethodsInstance.includes('Hello');
    arktest.assertEQ(result1, true);
    let result2 = testStringMethodsInstance.includes('世界');
    arktest.assertEQ(result2, true);
}

function testStringMethodsStartsWith(): void {
    let result1 = testStringMethodsInstance.startsWith('Hello');
    arktest.assertEQ(result1, true);
    let result2 = testStringMethodsInstance.startsWith('World');
    arktest.assertEQ(result2, false);
}

function testStringMethodsEndsWith(): void {
    let result1 = testStringMethodsInstance.endsWith('🌍');
    arktest.assertEQ(result1, true);
    let result2 = testStringMethodsInstance.endsWith('Hello');
    arktest.assertEQ(result2, false);
}

function testStringMethodsRepeat(): void {
    let result = testStringMethodsInstance.repeat(2);
    arktest.assertEQ(result.includes('Hello World 你好世界 🌍'), true);
}

function testStringMethodsUpperCase(): void {
    let result = testStringMethodsInstance.toUpperCase();
    arktest.assertEQ(result.includes('HELLO'), true);
}

function testStringMethodsLowerCase(): void {
    let result = testStringMethodsInstance.toLowerCase();
    arktest.assertEQ(result.includes('hello'), true);
}

function testStringMethodsTrim(): void {
    let result = testStringMethodsInstance.trim();
    arktest.assertEQ(result.length > 0, true);
}

function testAll(): void {
    testHugeString();
    testChineseString();
    testEmojiString();
    testSpecialCharsString();
    testMixedString();
    testEmptyString();
    testWhitespaceString();
    testStringMethodsSubstring();
    testStringMethodsCharAt();
    testStringMethodsIndexOf();
    testStringMethodsIncludes();
    testStringMethodsStartsWith();
    testStringMethodsEndsWith();
    testStringMethodsRepeat();
    testStringMethodsUpperCase();
    testStringMethodsLowerCase();
    testStringMethodsTrim();
}
