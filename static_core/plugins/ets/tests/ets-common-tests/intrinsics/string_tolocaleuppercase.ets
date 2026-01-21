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

function main() {
    let suite = new arktest.ArkTestsuite('StringToLocaleUpperCase');
    suite.addTest('testTurkishLocaleSpecific', testTurkishLocaleSpecific);
    suite.addTest('testEnglishLocaleSpecific', testEnglishLocaleSpecific);
    suite.addTest('testCompressedTurkishLocaleSpecific', testCompressedTurkishLocaleSpecific);
    suite.addTest('testCompressedEnglishLocaleSpecificLinearLocale',
        testCompressedEnglishLocaleSpecificLinearLocale);
    suite.addTest('testCompressedEnglishLocaleSpecificLinearShortLocale',
        testCompressedEnglishLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedEnglishLocaleSpecificTreeLocale',
        testCompressedEnglishLocaleSpecificTreeLocale);
    suite.addTest('testCompressedEnglishLocaleSpecificLinearSlicedLocale',
        testCompressedEnglishLocaleSpecificSlicedLocale);
    suite.addTest('testCompressedRussianLocaleSpecificLinearLocale',
        testCompressedRussianLocaleSpecificLinearLocale);
    suite.addTest('testCompressedRussianLocaleSpecificLinearShortLocale',
        testCompressedRussianLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedRussianLocaleSpecificTreeLocale',
        testCompressedRussianLocaleSpecificTreeLocale);
    suite.addTest('testCompressedRussianLocaleSpecificLinearSlicedLocale',
        testCompressedRussianLocaleSpecificSlicedLocale);
    suite.addTest('testCompressedFrenchLocaleSpecificLinearShortLocale',
        testCompressedFrenchLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedGermanLocaleSpecificLinearShortLocale',
        testCompressedGermanLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedDutchLocaleSpecificLinearShortLocale',
        testCompressedDutchLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedItalianLocaleSpecificLinearShortLocale',
        testCompressedItalianLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedSpainLocaleSpecificLinearShortLocale',
        testCompressedSpainLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedPortugueseLocaleSpecificLinearShortLocale',
        testCompressedPortugueseLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedMalayLocaleSpecificLinearShortLocale',
        testCompressedMalayLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedHindiLocaleSpecificLinearShortLocale',
        testCompressedHindiLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedHebrewLocaleSpecificLinearShortLocale',
        testCompressedHebrewLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedGreekLocaleSpecificLinearShortLocale',
        testCompressedGreekLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedLithuanianLocaleSpecificLinearShortLocale',
        testCompressedLithuanianLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedChineseLocaleSpecificLinearShortLocale',
        testCompressedChineseLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedChineseLocaleSpecificLinearLocale',
        testCompressedChineseLocaleSpecificLinearLocale);
    suite.addTest('testCompressedChineseLocaleSpecificTreeLocale',
        testCompressedChineseLocaleSpecificTreeLocale);
    suite.addTest('testCompressedChineseLocaleSpecificLinearSlicedLocale',
        testCompressedChineseLocaleSpecificSlicedLocale);
    suite.addTest('testShortCompressedEnglishLocaleSpecificLinearLocale',
        testShortCompressedEnglishLocaleSpecificLinearLocale);
    return suite.run();
}

/**
 * Tests Turkish locale-specific uppercase conversion rules with multiple string operations.
 *
 * Key Test Points:
 * 1. Verifies the Turkish-specific mappings:
 *    - 'i' → 'İ' (dotted uppercase I)
 *    - 'ı' → 'I' (dotless uppercase I)
 *    - 'ğşçöü' → 'ĞŞÇÖÜ'
 * 2. Validates non-Turkish characters:
 *    - 'σ' (Greek sigma) → 'Σ'
 *    - 'ß' (German sharp S) → 'SS'
 * 3. Tests string manipulation combinations:
 *    - Concatenation (String.prototype.concat)
 *    - Substring extraction (String.prototype.substring)
 */
function testTurkishLocaleSpecific() {
    // Base string test
    const input = 'iığşçöü12345!@#âσß';
    const expected = 'İIĞŞÇÖÜ12345!@#ÂΣSS';
    const result = input.toLocaleUpperCase('tr-TR');
    arktest.assertEQ(result, expected, `TR Locale Failed: ${result} vs ${expected}`);

    // TreeString test
    const input_0 = 'iığşçöü12';
    const input_1 = '345!@#âσß';
    const inputTree = input_0.concat(input_1);
    const resultTree = inputTree.toLocaleUpperCase('tr-TR');
    arktest.assertEQ(resultTree, expected, `TR Locale Failed: ${resultTree} vs ${expected}`);

    // SlicedString test
    const inputSliced = 'abciığşçöü12345!@#âσßdef'.substring(3, 21);
    const resultSliced = inputSliced.toLocaleUpperCase('tr-TR');
    arktest.assertEQ(resultSliced, expected, `TR Locale Failed: ${resultSliced} vs ${expected}`);
}

/**
 * Tests Turkish locale-specific uppercase conversion rules for compressible (basic-latin-only) strings.
 */
function testCompressedTurkishLocaleSpecific() {
    // Base string test
    const input = 'aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ1234567890./!?-+_:*\\\'"';
    arktest.assertEQ(input.length, 26 * 2 + 10 + 12);
    const expected = 'AABBCCDDEEFFGGHHİIJJKKLLMMNNOOPPQQRRSSTTUUVVWWXXYYZZ1234567890./!?-+_:*\\\'"';
    arktest.assertEQ(expected.length, 26 * 2 + 10 + 12);
    const result = input.toLocaleUpperCase('tr-TR');
    arktest.assertEQ(result, expected, `compressible TR Locale Failed: ${result} vs ${expected}`);

    // TreeString test
    const input_0 = 'abcdefghijklmnopqrstuvwxyz';
    const input_1 = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
    const input_2 = '1234567890./!?-+_:*\\\'"';
    const inputTree = input_0.concat(input_1).concat(input_2);
    arktest.assertEQ(inputTree.length, 26 * 2 + 10 + 12);
    const expectedTree = 'ABCDEFGHİJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890./!?-+_:*\\\'"';
    const resultTree = inputTree.toLocaleUpperCase('tr-TR');
    arktest.assertEQ(resultTree, expectedTree, `compressible tree TR Locale Failed: ${resultTree} vs ${expectedTree}`);

    // SlicedString test
    const inputSliced = '-*&abcdefghijklmnopqrstuvwxyz1234567890./!?-+_:&\\\'"='.substring(3, 26 + 3 + 10 + 12);
    const expectedSliced = 'ABCDEFGHİJKLMNOPQRSTUVWXYZ1234567890./!?-+_:&\\\'"';
    const resultSliced = inputSliced.toLocaleUpperCase('tr-TR');
    arktest.assertEQ(resultSliced, expectedSliced,
        `compressible sliced TR Locale Failed: ${resultSliced} vs ${expectedSliced}`);
}

/**
 * Tests English (US) locale uppercase conversion with Unicode normalization.
 *
 * Key Test Points:
 * 1. Verifies English-specific behavior:
 *    - 'i' → 'I' (normal uppercase I)
 *    - 'i̇' (i with combining dot) → 'İ'
 * 2. Handles Unicode normalization:
 *    - Normalizes results to NFC form for reliable comparison
 * 3. Tests multilingual characters:
 *    - 'ğşçöü' (Turkish) → 'ĞŞÇÖÜ'
 *    - 'σ' (Greek) → 'Σ'
 *    - 'ß' → 'SS'
 * 4. Validates string operations:
 *    - Concatenation and substring handling
 */
function testEnglishLocaleSpecific() {
    // Base string with combining characters
    const input = 'ii̇ğşçöü12345!@#âσß';
    const expected = 'IİĞŞÇÖÜ12345!@#ÂΣSS'.normalize('NFC');
    const result = input.toLocaleUpperCase('en-US').normalize('NFC');
    arktest.assertEQ(result, expected, `en-US Locale Failed: ${result} vs ${expected}`);

    // TreeString test with normalization
    const input_0 = 'ii̇ğşçöü1';
    const input_1 = '2345!@#âσß';
    const inputTree = input_0.concat(input_1);
    const resultTree = inputTree.toLocaleUpperCase('en-US').normalize('NFC');
    arktest.assertEQ(resultTree, expected, `en-US Locale Failed: ${resultTree} vs ${expected}`);

    // SlicedString test with normalization
    const inputSliced = 'abcii̇ğşçöü12345!@#âσßdef'.substring(3, 22);
    const resultSliced = inputSliced.toLocaleUpperCase('en-US').normalize('NFC');
    arktest.assertEQ(resultSliced, expected, `en-US Locale Failed: ${resultSliced} vs ${expected}`);
}

function getTreeLocale(lang: String, country: String) {
    arktest.assertEQ(lang.length, 2, 'The language code must have exactly 2 symbols');
    arktest.assertEQ(country.length, 2, 'The country code must have exactly 2 symbols');
    const locale = `${lang}-${country}`;
    arktest.assertEQ(locale.length, 5, `Locale length must be exactly 5 symbols, failed: ${locale}`);
    return locale;
}

function getSlicedLocale(lang: String, country: String) {
    arktest.assertEQ(lang.length, 2, 'The language code must have exactly 2 symbols');
    arktest.assertEQ(country.length, 2, 'The country code must have exactly 2 symbols');
    const data: String = `co${lang}-${country}Locale`;
    const locale = data.substring(2, 7);
    arktest.assertEQ(locale.length, 5, `Locale length must be exactly 5 symbols, failed: ${locale}`);
    return locale;
}

/**
 * Tests English (US) locale uppercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedEnglishLocaleSpecificLinearLocale() {
    testCompressedCompatibleLocaleSpecific('en-US');
}

function testCompressedEnglishLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('en');
}

function testCompressedEnglishLocaleSpecificTreeLocale() {
    testCompressedCompatibleLocaleSpecific(getTreeLocale('en', 'US'))
}

function testCompressedEnglishLocaleSpecificSlicedLocale() {
    testCompressedCompatibleLocaleSpecific(getSlicedLocale('en', 'US'));
}

/**
 * Tests english-compatible locale uppercase conversion for a short (less than 8 characters) compressible
 * (basic-latin-only) strings.
 */
function testShortCompressedEnglishLocaleSpecificLinearLocale() {
    const input1 = 'a';
    const input1Next = 'alphaNewString';
    const input1NextLen = input1Next.length;
    arktest.assertEQ(input1.length, 1);
    const expected1 = 'A';
    const result1 = input1.toLocaleUpperCase('en-US');
    arktest.assertEQ(result1, expected1, `Short Compressible en-US Locale Failed: ${result1} vs ${expected1}`);
    arktest.assertEQ(input1Next, 'alphaNewString', 'the next string must not be touched');
    arktest.assertEQ(input1Next.length, input1NextLen, 'the next string must not be touched including the length');

    const input7 = 'abcdefg';
    const input7Next = 'betaNewString';
    const input7NextLen = input7Next.length;
    arktest.assertEQ(input7.length, 7);
    const expected7 = 'ABCDEFG';
    const result7 = input7.toLocaleUpperCase('en-US');
    arktest.assertEQ(result7, expected7, `Short Compressible en-US Locale Failed: ${result7} vs ${expected7}`);
    arktest.assertEQ(input7Next, 'betaNewString', 'the next string must not be touched');
    arktest.assertEQ(input7Next.length, input7NextLen, 'the next string must not be touched including the length');

    const input8 = 'abcdefgh';
    const input8Next = 'gammaNewString';
    const input8NextLen = input8Next.length;
    arktest.assertEQ(input8.length, 8);
    const expected8 = 'ABCDEFGH';
    const result8 = input8.toLocaleUpperCase('en-US');
    arktest.assertEQ(result8, expected8, `Short Compressible en-US Locale Failed: ${result8} vs ${expected8}`);
    arktest.assertEQ(input8Next, 'gammaNewString', 'the next string must not be touched');
    arktest.assertEQ(input8Next.length, input8NextLen, 'the next string must not be touched including the length');
}

/**
 * Test Russian (RU) locale uppercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedRussianLocaleSpecificLinearLocale() {
    testCompressedCompatibleLocaleSpecific('ru-RU');
}

function testCompressedRussianLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('ru');
}

function testCompressedRussianLocaleSpecificTreeLocale() {
    testCompressedCompatibleLocaleSpecific(getTreeLocale('ru', 'RU'));
}

function testCompressedRussianLocaleSpecificSlicedLocale() {
    testCompressedCompatibleLocaleSpecific(getSlicedLocale('ru', 'RU'));
}

/**
 * Test French locale uppercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedFrenchLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('fr');
}

/**
 * Test German locale uppercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedGermanLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('de');
}

/**
 * Test Dutch locale uppercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedDutchLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('nl');
}

/**
 * Test Hindi locale uppercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedHindiLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('hi');
}

/**
 * Test Hebrew locale uppercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedHebrewLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('he');
}

/**
 * Test Greek locale uppercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedGreekLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('el');
}

/**
 * Test Lithuanian locale uppercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedLithuanianLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('lt');
}

/**
 * Test Italian locale uppercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedItalianLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('it');
}

/**
 * Test Spain locale uppercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedSpainLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('es');
}

/**
 * Test Portuguese locale uppercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedPortugueseLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('pt');
}

/**
 * Test Malay locale uppercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedMalayLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('ms');
}

/**
 * Tests Chinese (zh-CN) locale uppercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedChineseLocaleSpecificLinearLocale() {
    testCompressedCompatibleLocaleSpecific('zh-CN');
}

function testCompressedChineseLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('zh');
}

function testCompressedChineseLocaleSpecificTreeLocale() {
    testCompressedCompatibleLocaleSpecific(getTreeLocale('zh', 'CN'))
}

function testCompressedChineseLocaleSpecificSlicedLocale() {
    testCompressedCompatibleLocaleSpecific(getSlicedLocale('zh', 'CN'));
}

/**
 * Tests english-compatible locale uppercase conversion for compressible (basic-latin-only) strings.
 * Important: the locale code may also be a tree or even slice string.
 */
function testCompressedCompatibleLocaleSpecific(locale: String) {
    // Base compressible string characters
    const input = ' aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ1234567890./!?-+_:*\\\'"`(){}[]';
    arktest.assertEQ(input.length, 26 * 2 + 10 + 20);
    const expected = ' AABBCCDDEEFFGGHHIIJJKKLLMMNNOOPPQQRRSSTTUUVVWWXXYYZZ1234567890./!?-+_:*\\\'"`(){}[]';
    arktest.assertEQ(expected.length, 26 * 2 + 10 + 20);
    const result = input.toLocaleUpperCase(locale);
    arktest.assertEQ(result, expected, `Compressible ${locale} Locale Failed: ${result} vs ${expected}`);

    // TreeString test
    const input_0 = 'abcdefghijklmnopqrstuvwxyz';
    const input_1 = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
    const input_2 = ' 1234567890./!?-+_:*\\\'"`(){}[]';
    const inputTree = input_0.concat(input_1).concat(input_2);
    arktest.assertEQ(inputTree.length, 26 * 2 + 10 + 20);
    const expectedTree = 'ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890./!?-+_:*\\\'"`(){}[]';
    const resultTree = inputTree.toLocaleUpperCase(locale);
    arktest.assertEQ(resultTree, expectedTree,
        `Compressible Tree ${locale} Locale Failed: ${resultTree} vs ${expectedTree}`);

    // SlicedString test
    const inputSliced = '-*& abcdefghijklmnopqrstuvwxyz1234567890./!?-+_:&\\\'"=`(){}[]x'
        .substring(3, 26 + 3 + 10 + 21);
    arktest.assertEQ(inputSliced.length, 26 + 10 + 21);
    const expectedSliced = ' ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890./!?-+_:&\\\'"=`(){}[]';
    arktest.assertEQ(expectedSliced.length, 26 + 10 + 21);
    const resultSliced = inputSliced.toLocaleUpperCase(locale);
    arktest.assertEQ(resultSliced, expectedSliced,
        `Compressible Sliced ${locale} Locale Failed: ${resultSliced} vs ${expectedSliced}`);
}
