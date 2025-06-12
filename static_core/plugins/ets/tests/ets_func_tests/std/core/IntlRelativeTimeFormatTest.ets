/**
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

const fakeLocale = "er-or"

function main() throws {
    testFormat()
    testFormatToParts()
    testResolvedOptions()
    testSelectWithOptions()
    testSupportedLocalesOf()
}

function runFormatTest(locale: Intl.UnicodeBCP47LocaleIdentifier, value: number, unit: Intl.RelativeTimeFormatUnit, expected: string, options?: Intl.RelativeTimeFormatOptions) {
    const format = new Intl.RelativeTimeFormat(locale, options)
    const result = format.format(value, unit)
    assertEQ(result, expected)
}

function testFormat() {
    runFormatTest("en", 1, "day", "tomorrow", { numeric: "auto" })
    runFormatTest("en", -1, "day", "yesterday", { numeric: "auto" })
    runFormatTest("ru", 2, "days", "послезавтра", { numeric: "auto" })
    runFormatTest("ru", 0, "day", "сегодня", { numeric: "always" })
    runFormatTest("fr", 5, "month", "dans 5 mois", { style: "long" })
    runFormatTest("fr", -5, "month", "il y a 5 mois", { style: "long" })
}

function runFormatToPartsTest(locale: string, value: number, unit: Intl.RelativeTimeFormatUnit, expected: string, options?: Intl.RelativeTimeFormatOptions) {
    const format = new Intl.RelativeTimeFormat(locale, options)
    const result = format.formatToParts(value, unit)
    arktest.assertEQ(result[0].value, expected)
}

function testFormatToParts() {
    runFormatToPartsTest('zh-CN', 3, 'day', '3天后', { numeric: 'auto' })
    runFormatToPartsTest('zh-CN', 2, 'day', '后天', { numeric: 'auto' })
    runFormatToPartsTest('zh-CN', 1, 'day', '明天', { numeric: 'auto' })
    runFormatToPartsTest('zh-CN', 0, 'day', '今天', { numeric: 'auto' })
    runFormatToPartsTest('zh-CN', -1, 'day', '昨天', { numeric: 'auto' })
    runFormatToPartsTest('zh-CN', -2, 'day', '前天', { numeric: 'auto' })
    runFormatToPartsTest('zh-CN', -3, 'day', '3天前', { numeric: 'auto' })

    runFormatToPartsTest('en', 3, 'day', 'in 3 days', { numeric: 'auto' })
    runFormatToPartsTest('en', 2, 'day', 'in 2 days', { numeric: 'auto' })
    runFormatToPartsTest('en', 1, 'day', 'tomorrow', { numeric: 'auto' })
    runFormatToPartsTest('en', 0, 'day', 'today', { numeric: 'auto' })
    runFormatToPartsTest('en', -1, 'day', 'yesterday', { numeric: 'auto' })
    runFormatToPartsTest('en', -2, 'day', '2 days ago', { numeric: 'auto' })
    runFormatToPartsTest('en', -3, 'day', '3 days ago', { numeric: 'auto' })

    runFormatToPartsTest('ru', 3, 'day', 'через 3 дня', { numeric: 'auto' })
    runFormatToPartsTest('ru', 2, 'day', 'послезавтра', { numeric: 'auto' })
    runFormatToPartsTest('ru', 1, 'day', 'завтра', { numeric: 'auto' })
    runFormatToPartsTest('ru', 0, 'day', 'сегодня', { numeric: 'auto' })
    runFormatToPartsTest('ru', -1, 'day', 'вчера', { numeric: 'auto' })
    runFormatToPartsTest('ru', -2, 'day', 'позавчера', { numeric: 'auto' })
    runFormatToPartsTest('ru', -3, 'day', '3 дня назад', { numeric: 'auto' })

    runFormatToPartsTest('fr', 3, 'day', 'dans 3 jours', { numeric: 'auto' })
    runFormatToPartsTest('fr', 2, 'day', 'après-demain', { numeric: 'auto' })
    runFormatToPartsTest('fr', 1, 'day', 'demain', { numeric: 'auto' })
    runFormatToPartsTest('fr', 0, 'day', 'aujourd’hui', { numeric: 'auto' })
    runFormatToPartsTest('fr', -1, 'day', 'hier', { numeric: 'auto' })
    runFormatToPartsTest('fr', -2, 'day', 'avant-hier', { numeric: 'auto' })
    runFormatToPartsTest('fr', -3, 'day', 'il y a 3 jours', { numeric: 'auto' })
}

function testResolvedOptions() {
    {
        const format = new Intl.RelativeTimeFormat("en")
        const result = format.resolvedOptions()
        console.log(result)
        assertEQ(result.style, "long")
        assertEQ(result.numeric, "always")
        assertEQ(result.numberingSystem, "latn")
    }
    {
        const format = new Intl.RelativeTimeFormat("en", { style: "short" } as Intl.RelativeTimeFormatOptions)
        const result = format.resolvedOptions()
        assertEQ(result.style, "short")
    }
    {
        const format = new Intl.RelativeTimeFormat("en", { numeric: "always" } as Intl.RelativeTimeFormatOptions)
        const result = format.resolvedOptions()
        assertEQ(result.numeric, "always")
    }
}

function runTestWithOptions(locale: Intl.UnicodeBCP47LocaleIdentifier, value: number, unit: Intl.RelativeTimeFormatUnit, options: Intl.RelativeTimeFormatOptions, expected: string) {
    const format = new Intl.RelativeTimeFormat(locale, options)
    const result = format.format(value, unit)
    assertEQ(result, expected)
}

function testSelectWithOptions() {
    runTestWithOptions("en", 1, "day", { numeric: "auto" }, "tomorrow")
    runTestWithOptions("en", -1, "hour", { numeric: "auto" }, "1 hour ago")
    runTestWithOptions("en", 0, "minute", { numeric: "always" }, "this minute")
    runTestWithOptions("ru", 3, "month", { style: "long" }, "через 3 месяца")
    runTestWithOptions("fr", 7, "day", { style: "short" }, "dans 7 jours")
    runTestWithOptions("zh", -7, "day", { style: "short" }, "7天前")
}

function runSupportedLocalesStringTest(locales: string, options: Intl.RelativeTimeFormatOptions | undefined, expected: string[]) {
    const result = Intl.RelativeTimeFormat.supportedLocalesOf(locales, options)
    assertEQ(result.length, expected.length)
    for (let i: int = 0; i < result.length; i++) {
        const isEq = result[i].includes(expected[i])
        assertTrue(isEq);
    }
}

function runSupportedLocalesArrayTest(locales: string[], options: Intl.RelativeTimeFormatOptions | undefined, expected: string[]) {
    const result = Intl.RelativeTimeFormat.supportedLocalesOf(locales, options)
    assertEQ(result.length, expected.length)
    for (let i: int = 0; i < result.length; i++) {
        const isEq = result[i].includes(expected[i])
        assertTrue(isEq);
    }
}

function testSupportedLocalesOf() {
    runSupportedLocalesStringTest(
        "en",
        undefined,
        [
            intlBestFitLocale("en")
        ]
    )
    runSupportedLocalesStringTest(
        fakeLocale,
        undefined,
        [
            intlBestFitLocale(fakeLocale)
        ]
    )
    runSupportedLocalesStringTest(
        "en",
        { localeMatcher: "lookup" } as Intl.RelativeTimeFormatOptions,
        [
            intlLookUpLocale("en")
        ]
    )
    runSupportedLocalesArrayTest(
        ["fr", "ru"],
        undefined,
        [
            intlBestFitLocale("fr"),
            intlBestFitLocale("ru")
        ]
    )
    runSupportedLocalesArrayTest(
        ["ru-RU", "zh-CN", "en-US"],
        undefined,
        [
            intlBestFitLocale("ru-RU"),
            intlBestFitLocale("zh-CN"),
            intlBestFitLocale("en-US")
        ]
    )
    runSupportedLocalesArrayTest(
        ["ru-RU", "zh-CN", "en-US"],
        { localeMatcher: "best fit" } as Intl.RelativeTimeFormatOptions,
        [
            intlBestFitLocale("ru-RU"),
            intlBestFitLocale("zh-CN"),
            intlBestFitLocale("en-US")
        ]
    )

    runSupportedLocalesArrayTest(
        ["ru-RU", "en-US"],
        { localeMatcher: "lookup" } as Intl.RelativeTimeFormatOptions,
        [
            intlLookUpLocale("ru-RU"),
            intlLookUpLocale("en-US")
        ]
    )
}
