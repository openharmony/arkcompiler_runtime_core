/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

const en_US: string = intlBestFitLocale("en-US");
const zh_CN: string = intlBestFitLocale("zh-CN");
const ru_RU = intlBestFitLocale("ru-RU");
const er_ER = "er-OR" // Fake locale
const localeList: string[] = [er_ER, ru_RU, zh_CN, en_US];

function main() throws {
    // NOTE(igorlegalov): uncomment when the issue #25344 is fixed
    // testBestFitLocaleMatchList()
    testLookupLocaleMatchSingle()
    testLookupLocaleMatchList()
    negativeTestLanguageTag()
}

function testBestFitLocaleMatchList() {
    const result = intlBestFitLocale(localeList);
    // First existing locale should be selected
    arktest.assertEQ(result, ru_RU)
}

function testLookupLocaleMatchSingle() {
    const result = intlLookUpLocale(en_US);
    arktest.assertEQ(result, en_US)
}

function testLookupLocaleMatchList() {
    let result = intlLookUpLocale(localeList);
    // First existing locale should be selected
    arktest.assertEQ(result, ru_RU)
}

function negativeTestLanguageTag() {
    const err = new RangeError("Incorrect locale information provided")
    expectError((): void => { intlBestFitLocale("test") }, err)
    expectError((): void => { intlLookUpLocale("test") }, err)
}