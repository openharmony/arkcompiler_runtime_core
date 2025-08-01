/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

const en_US: string = "en-US"
const zh_CN: string = "zh-CN"
const ru_RU = "ru-RU"
const er_ER = "er-OR" // Fake locale
const badLocale = "test"
const localeList: string[] = [er_ER, ru_RU, zh_CN, en_US]

function main() throws {
    testBestFitLocaleMatchList()
    testLookupLocaleMatchSingle()
    testLookupLocaleMatchList()
    negativeTestLanguageTag()
}

function testBestFitLocaleMatchList() {
    arktest.assertEQ(intlBestFitLocale(localeList), ru_RU)

    const result = intlBestFitLocales(localeList)
    arktest.assertEQ(result.length, 3)
    arktest.assertEQ(result[0], ru_RU)
    arktest.assertEQ(result[1], zh_CN)
    arktest.assertEQ(result[2], en_US)
}

function testLookupLocaleMatchSingle() {
    arktest.assertEQ(intlLookUpLocale(en_US), en_US)

    const result = intlLookUpLocales(en_US)
    arktest.assertEQ(result.length, 1)
    arktest.assertEQ(result[0], en_US)
}

function testLookupLocaleMatchList() {
    arktest.assertEQ(intlLookUpLocale(localeList), ru_RU)

    let result = intlLookUpLocales(localeList)
    arktest.assertEQ(result.length, 3)
    arktest.assertEQ(result[0], ru_RU)
    arktest.assertEQ(result[1], zh_CN)
    arktest.assertEQ(result[2], en_US)
}

function negativeTestLanguageTag() {
    expectError((): void => { intlBestFitLocale(badLocale) })
    expectError((): void => { intlBestFitLocales(badLocale) })
    expectError((): void => { intlBestFitLocales([badLocale]) })
    expectError((): void => { intlLookUpLocale(badLocale) })
    expectError((): void => { intlLookUpLocales(badLocale) })
    expectError((): void => { intlLookUpLocales([badLocale]) })
}