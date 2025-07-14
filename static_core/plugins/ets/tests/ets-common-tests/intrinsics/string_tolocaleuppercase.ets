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

function main() {
    let suite = new arktest.ArkTestsuite('StringToLocaleUpperCase');
    suite.addTest('testTurkishLocaleSpecific', testTurkishLocaleSpecific);
    suite.addTest('testEnglishLocaleSpecific', testEnglishLocaleSpecific);
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
    arktest.assertTrue(result === expected,
        `TR Locale Failed: ${result} vs ${expected}`);

    // TreeString test
    const input_0 = 'iığşçöü12';
    const input_1 = '345!@#âσß';
    const inputTree = input_0.concat(input_1);
    const resultTree = inputTree.toLocaleUpperCase('tr-TR');
    arktest.assertTrue(resultTree === expected,
        `TR Locale Failed: ${resultTree} vs ${expected}`);

    // SlicedString test
    const inputSliced = 'abciığşçöü12345!@#âσßdef'.substring(3, 21);
    const resultSliced = inputSliced.toLocaleUpperCase('tr-TR');
    arktest.assertTrue(resultSliced === expected,
        `TR Locale Failed: ${resultSliced} vs ${expected}`);
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
    arktest.assertTrue(result === expected,
        `en-US Locale Failed: ${result} vs ${expected}`);

    // TreeString test with normalization
    const input_0 = 'ii̇ğşçöü1';
    const input_1 = '2345!@#âσß';
    const inputTree = input_0.concat(input_1);
    const resultTree = inputTree.toLocaleUpperCase('en-US').normalize('NFC');
    arktest.assertTrue(resultTree === expected,
        `en-us Locale Failed: ${resultTree} vs ${expected}`);

    // SlicedString test with normalization
    const inputSliced = 'abcii̇ğşçöü12345!@#âσßdef'.substring(3, 22);
    const resultSliced = inputSliced.toLocaleUpperCase('en-US').normalize('NFC');
    arktest.assertTrue(resultSliced === expected,
        `en-us Locale Failed: ${resultSliced} vs ${expected}`);
}
