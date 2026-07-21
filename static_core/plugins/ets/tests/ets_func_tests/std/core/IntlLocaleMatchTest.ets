/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

const enUS: string = 'en-US'
const zhCN: string = 'zh-CN'
const ruRU = 'ru-RU'
const erER = 'er-OR' // Fake locale
const badLocale = 'test'
const localeList: string[] = [erER, ruRU, zhCN, enUS]

function main() {
    let suite = new arktest.ArkTestsuite('Locale matching algos')
    suite.addTest('Bestfit list locales', testBestFitLocaleMatchList)
    suite.addTest('Lookup locale', testLookupLocaleMatchSingle)
    suite.addTest('Lookup list locales', testLookupLocaleMatchList)
    suite.addTest('negitve test for locale match', negativeTestLanguageTag)
    return suite.run()
}

function testBestFitLocaleMatchList() {
    arktest.assertEQ(Intl.intlBestFitLocale(localeList), ruRU)

    const result = Intl.intlBestFitLocales(localeList)
    arktest.assertEQ(result.length, 3)
    arktest.assertEQ(result[0], ruRU)
    arktest.assertEQ(result[1], zhCN)
    arktest.assertEQ(result[2], enUS)
}

function testLookupLocaleMatchSingle() {
    arktest.assertEQ(Intl.intlLookUpLocale(enUS), enUS)

    const result = Intl.intlLookUpLocales(enUS)
    arktest.assertEQ(result.length, 1)
    arktest.assertEQ(result[0], enUS)
}

function testLookupLocaleMatchList() {
    arktest.assertEQ(Intl.intlLookUpLocale(localeList), ruRU)

    let result = Intl.intlLookUpLocales(localeList)
    arktest.assertEQ(result.length, 3)
    arktest.assertEQ(result[0], ruRU)
    arktest.assertEQ(result[1], zhCN)
    arktest.assertEQ(result[2], enUS)
}

function negativeTestLanguageTag() {
    arktest.expectError((): void => { Intl.intlBestFitLocale(badLocale) })
    arktest.expectError((): void => { Intl.intlBestFitLocales(badLocale) })
    arktest.expectError((): void => { Intl.intlBestFitLocales([badLocale]) })
    arktest.expectError((): void => { Intl.intlLookUpLocale(badLocale) })
    arktest.expectError((): void => { Intl.intlLookUpLocales(badLocale) })
    arktest.expectError((): void => { Intl.intlLookUpLocales([badLocale]) })
}