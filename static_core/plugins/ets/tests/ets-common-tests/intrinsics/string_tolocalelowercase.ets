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
    let suite = new arktest.ArkTestsuite("StringToLocaleLowerCase");
    suite.addTest('testTurkishLocaleSpecific', testTurkishLocaleSpecific);
    suite.addTest('testEnglishLocaleSpecific', testEnglishLocaleSpecific);
    return suite.run();
}

// Test Case 1: Turkish-specific rules with mixed characters
// Input: 'İIĞŞÇÖÜ12345!@#ÂΣẞ' (length: 18)
// Locale: 'tr-TR'
// Expected Output: 'iığşçöü12345!@#âσß'
function testTurkishLocaleSpecific() {
    const input = 'İIĞŞÇÖÜ12345!@#ÂΣẞ';
    const expected = 'iığşçöü12345!@#âσß';
    const result = input.toLocaleLowerCase('tr-TR');
    arktest.assertTrue(result === expected,
        `TR Locale Failed: ${result} vs ${expected}`);

    const input_0 = new String('İIĞŞÇÖÜ1');
    const input_1 = new String('2345!@#ÂΣẞ');
    const inputTree = input_0.concat(input_1);
    arktest.assertTrue(inputTree.toLocaleLowerCase('tr-TR') === expected,
        `TR Locale Failed: ${inputTree.toLocaleLowerCase('tr-TR')} vs ${expected}`);

    const inputSliced = new String('abcİIĞŞÇÖÜ12345!@#ÂΣẞdef').substring(3, 21);
    arktest.assertTrue(inputSliced.toLocaleLowerCase('tr-TR') === expected,
        `TR Locale Failed: ${inputSliced.toLocaleLowerCase('tr-TR')} vs ${expected}`);
}

// Test Case 2: English locale with Unicode normalization
// Input: 'IİĞŞÇÖÜ12345!@#ÂΣẞ' (length: 17)
// Locale: 'en-US'
// Expected Output: 'i\u0069\u0307ğşçöü12345!@#âσß' 
function testEnglishLocaleSpecific() {
    const input = 'IİĞŞÇÖÜ12345!@#ÂΣẞ';
    const expected = 'ii̇ğşçöü12345!@#âσß';
    const result = input.toLocaleLowerCase('en-US');
    arktest.assertTrue(result === expected,
        `en-US Locale Failed: ${result} vs ${expected}`);

    const input_0 = new String('IİĞŞÇÖÜ1');
    const input_1 = new String('2345!@#ÂΣẞ');
    const inputTree = input_0.concat(input_1);
    arktest.assertTrue(result === inputTree.toLocaleLowerCase('en-US'));
    arktest.assertTrue(inputTree.toLocaleLowerCase('en-US') === expected,
        `en-US Locale Failed: ${inputTree.toLocaleLowerCase('en-US')} vs ${expected}`);

    const inputSliced = new String('abcIİĞŞÇÖÜ12345!@#ÂΣẞdef').substring(3, 21);
    arktest.assertTrue(inputSliced.toLocaleLowerCase('en-US') === expected,
        `en-US Locale Failed: ${inputSliced.toLocaleLowerCase('en-US')} vs ${expected}`);
}
