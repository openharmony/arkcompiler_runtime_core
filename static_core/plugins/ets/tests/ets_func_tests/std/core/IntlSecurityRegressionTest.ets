/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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
 * Regression tests for security fixes in intl native code.
 *
 * Alert #7: StdCoreIntlBestFitLocale / StdCoreIntlLookupLocale —
 *   ThrowRangeError after invalid locale tag must return nullptr
 *   (previously continued execution → FATAL abort on 2nd invalid tag).
 *   Before fix: passing >=2 invalid tags caused process abort (DoS).
 *   After fix: RangeError is thrown gracefully.
 */

function testBestFitLocaleInvalidTagSingle() {
    try {
        Intl.intlBestFitLocale(['xx-invalid-tag1'])
        arktest.assertTrue(false, 'should throw RangeError for invalid locale tag')
    } catch (e) {
        arktest.assertTrue(e instanceof RangeError, 'expected RangeError, got ' + e.name)
    }
}

function testBestFitLocaleMultipleInvalidTags() {
    try {
        Intl.intlBestFitLocale(['xx-invalid-tag1', 'xx-invalid-tag2'])
        arktest.assertTrue(false, 'should throw RangeError for multiple invalid locale tags')
    } catch (e) {
        arktest.assertTrue(e instanceof RangeError, 'expected RangeError, got ' + e.name)
    }
}

function testLookupLocaleInvalidTagSingle() {
    try {
        Intl.intlLookUpLocale(['xx-invalid-tag1'])
        arktest.assertTrue(false, 'should throw RangeError for invalid locale tag')
    } catch (e) {
        arktest.assertTrue(e instanceof RangeError, 'expected RangeError, got ' + e.name)
    }
}

function testLookupLocaleMultipleInvalidTags() {
    try {
        Intl.intlLookUpLocale(['xx-invalid-tag1', 'xx-invalid-tag2'])
        arktest.assertTrue(false, 'should throw RangeError for multiple invalid locale tags')
    } catch (e) {
        arktest.assertTrue(e instanceof RangeError, 'expected RangeError, got ' + e.name)
    }
}

function testBestFitLocalesMultipleInvalidTags() {
    try {
        Intl.intlBestFitLocales(['xx-invalid-tag1', 'xx-invalid-tag2'])
        arktest.assertTrue(false, 'should throw RangeError for multiple invalid locale tags')
    } catch (e) {
        arktest.assertTrue(e instanceof RangeError, 'expected RangeError, got ' + e.name)
    }
}

function testLookupLocalesMultipleInvalidTags() {
    try {
        Intl.intlLookUpLocales(['xx-invalid-tag1', 'xx-invalid-tag2'])
        arktest.assertTrue(false, 'should throw RangeError for multiple invalid locale tags')
    } catch (e) {
        arktest.assertTrue(e instanceof RangeError, 'expected RangeError, got ' + e.name)
    }
}

function testValidLocaleStillWorks() {
    let tag = Intl.intlBestFitLocale(['en-US'])
    arktest.assertTrue(tag.length > 0, 'valid locale should return non-empty tag')
    let tags = Intl.intlBestFitLocales(['en-US', 'zh-CN'])
    arktest.assertTrue(tags.length >= 1, 'valid locales should return non-empty array')
}

function main() {
    let myTestsuite = new arktest.ArkTestsuite('IntlSecurityRegressionTest')
    myTestsuite.addTest('testBestFitLocaleInvalidTagSingle', testBestFitLocaleInvalidTagSingle)
    myTestsuite.addTest('testBestFitLocaleMultipleInvalidTags', testBestFitLocaleMultipleInvalidTags)
    myTestsuite.addTest('testLookupLocaleInvalidTagSingle', testLookupLocaleInvalidTagSingle)
    myTestsuite.addTest('testLookupLocaleMultipleInvalidTags', testLookupLocaleMultipleInvalidTags)
    myTestsuite.addTest('testBestFitLocalesMultipleInvalidTags', testBestFitLocalesMultipleInvalidTags)
    myTestsuite.addTest('testLookupLocalesMultipleInvalidTags', testLookupLocalesMultipleInvalidTags)
    myTestsuite.addTest('testValidLocaleStillWorks', testValidLocaleStillWorks)
    return myTestsuite.run()
}
