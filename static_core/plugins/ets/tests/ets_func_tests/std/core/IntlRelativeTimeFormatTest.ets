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
        intlBestFitLocales("en")
    )
    runSupportedLocalesStringTest(
        fakeLocale,
        undefined,
        intlBestFitLocales(fakeLocale)
    )
    runSupportedLocalesStringTest(
        "en",
        { localeMatcher: "lookup" } as Intl.RelativeTimeFormatOptions,
        intlLookUpLocales("en")
    )
    runSupportedLocalesArrayTest(
        ["fr", "ru"],
        undefined,
        intlBestFitLocales(["fr", "ru"])
    )
    runSupportedLocalesArrayTest(
        ["ru-RU", "zh-CN", "en-US"],
        undefined,
        intlBestFitLocales(["ru-RU", "zh-CN", "en-US"])
    )
    runSupportedLocalesArrayTest(
        ["ru-RU", "zh-CN", "en-US"],
        { localeMatcher: "best fit" } as Intl.RelativeTimeFormatOptions,
        intlBestFitLocales(["ru-RU", "zh-CN", "en-US"])
    )

    runSupportedLocalesArrayTest(
        ["ru-RU", "en-US"],
        { localeMatcher: "lookup" } as Intl.RelativeTimeFormatOptions,
        intlLookUpLocales(["ru-RU", "en-US"])
    )
}
