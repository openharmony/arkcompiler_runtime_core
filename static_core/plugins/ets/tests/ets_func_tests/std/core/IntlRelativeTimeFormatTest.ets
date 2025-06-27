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

function runFormatToPartsTest(locale: string, values: FixedArray<number>, unit: string, expected: FixedArray<string>,
                              options: Intl.RelativeTimeFormatOptions) {
    const format = new Intl.RelativeTimeFormat(locale, options);
    for (let j = 0; j < values.length; j++) {
        let result = format.formatToParts(values[j], unit as Intl.RelativeTimeFormatUnit);
        arktest.assertEQ(result[0].value, expected[j]);
    }
}

function formatToPartsForSencodTest() {
    const options: Intl.RelativeTimeFormatOptions = {numeric: 'always', style: 'long', localeMatcher: 'best fit'};
    const units: FixedArray<string> = ['second', 'seconds'];
    const values: FixedArray<number> = [0, 1, 100, -1, -100];
    for (let i = 0; i < units.length; i++) {
        let expected: FixedArray<string> = ['现在', '1秒钟后', '100秒钟后', '1秒钟前', '100秒钟前'];
        runFormatToPartsTest('zh', values, units[i], expected, options);

        expected = ['сейчас', 'через 1 секунду', 'через 100 секунд', '1 секунду назад', '100 секунд назад'];
        runFormatToPartsTest('ru', values, units[i], expected, options);

        expected = ['now', 'in 1 second', 'in 100 seconds', '1 second ago', '100 seconds ago'];
        runFormatToPartsTest('en', values, units[i], expected, options);

        expected = ['maintenant', 'dans 1 seconde', 'dans 100 secondes', 'il y a 1 seconde', 'il y a 100 secondes'];
        runFormatToPartsTest('fr', values, units[i], expected, options);
    }
}

function formatToPartsForMinuteTest() {
    const options: Intl.RelativeTimeFormatOptions = {numeric: 'always', style: 'short', localeMatcher: 'best fit'};
    const units: FixedArray<string> = ['minute', 'minutes'];
    const values: FixedArray<number> = [0, 1, 100, -1, -100];
    for (let i = 0; i < units.length; i++) {
        let expected: FixedArray<string> = ['此刻', '1分钟后', '100分钟后', '1分钟前', '100分钟前'];
        runFormatToPartsTest('zh', values, units[i], expected, options);

        expected = ['в эту минуту', 'через 1 минуту', 'через 100 минут', '1 минуту назад', '100 минут назад'];
        runFormatToPartsTest('ru', values, units[i], expected, options);

        expected = ['this minute', 'in 1 minute', 'in 100 minutes', '1 minute ago', '100 minutes ago'];
        runFormatToPartsTest('en', values, units[i], expected, options);

        expected = ['cette minute-ci', 'dans 1 minute', 'dans 100 minutes', 'il y a 1 minute', 'il y a 100 minutes'];
        runFormatToPartsTest('fr', values, units[i], expected, options);
    }
}

function formatToPartsForHourTest() {
    const options: Intl.RelativeTimeFormatOptions = {numeric: 'always', style: 'narrow', localeMatcher: 'best fit'};
    const units: FixedArray<string> = ['hour', 'hours'];
    const values: FixedArray<number> = [0, 1, 25, -1, -25];
    for (let i = 0; i < units.length; i++) {
        let expected: FixedArray<string> = ['这一时间 / 此时', '1小时后', '25小时后', '1小时前', '25小时前'];
        runFormatToPartsTest('zh', values, units[i], expected, options);

        expected = ['в этот час', 'через 1 час', 'через 25 часов', '1 час назад', '25 часов назад'];
        runFormatToPartsTest('ru', values, units[i], expected, options);

        expected = ['this hour', 'in 1 hour', 'in 25 hours', '1 hour ago', '25 hours ago'];
        runFormatToPartsTest('en', values, units[i], expected, options);

        expected = ['cette heure-ci', 'dans 1 heure', 'dans 25 heures', 'il y a 1 heure', 'il y a 25 heures'];
        runFormatToPartsTest('fr', values, units[i], expected, options);
    }
}

function formatToPartsForDayTest() {
    const options: Intl.RelativeTimeFormatOptions = {numeric: 'always', style: 'long', localeMatcher: 'lookup'};
    const units: FixedArray<string> = ['day', 'days'];
    const values: FixedArray<number> = [0, 1, 2, 3, -1, -2, -3];
    for (let i = 0; i < units.length; i++) {
        let expected: FixedArray<string> = ['今天', '明天', '后天', '3天后', '昨天', '前天', '3天前'];
        runFormatToPartsTest('zh', values, units[i], expected, options);

        expected = ['сегодня', 'завтра', 'послезавтра', 'через 3 дня', 'вчера', 'позавчера',  '3 дня назад'];
        runFormatToPartsTest('ru', values, units[i], expected, options);

        expected = ['today', 'tomorrow', 'in 2 days', 'in 3 days', 'yesterday', '2 days ago', '3 days ago'];
        runFormatToPartsTest('en', values, units[i], expected, options);

        expected = ['aujourd’hui', 'demain', 'après-demain', 'dans 3 jours', 'hier', 'avant-hier', 'il y a 3 jours'];
        runFormatToPartsTest('fr', values, units[i], expected, options);
    }
}

function formatToPartsForWeekTest() {
    const options: Intl.RelativeTimeFormatOptions = {numeric: 'auto', style: 'long', localeMatcher: 'best fit'};
    const units: FixedArray<string> = ['week', 'weeks'];
    const values: FixedArray<number> = [0, 1, 2, -1, -2];
    for (let i = 0; i < units.length; i++) {
        let expected: FixedArray<string> = ['本周', '下周', '2周后', '上周', '2周前'];
        runFormatToPartsTest('zh', values, units[i], expected, options);

        expected = ['на этой неделе', 'на следующей неделе', 'через 2 недели', 'на прошлой неделе', '2 недели назад'];
        runFormatToPartsTest('ru', values, units[i], expected, options);

        expected = ['this week', 'next week', 'in 2 weeks', 'last week', '2 weeks ago'];
        runFormatToPartsTest('en', values, units[i], expected, options);

        expected = ['cette semaine', 'la semaine prochaine', 'dans 2 semaines', 'la semaine dernière', 'il y a 2 semaines'];
        runFormatToPartsTest('fr', values, units[i], expected, options);
    }
}

function formatToPartsForMonthTest() {
    const options: Intl.RelativeTimeFormatOptions = {numeric: 'auto', style: 'short', localeMatcher: 'best fit'};
    const units: FixedArray<string> = ['month', 'months'];
    const values: FixedArray<number> = [0, 1, 2, -1, -2];
    for (let i = 0; i < units.length; i++) {
        let expected: FixedArray<string> = ['本月', '下个月', '2个月后', '上个月', '2个月前'];
        runFormatToPartsTest('zh', values, units[i], expected, options);

        expected = ['в этом месяце', 'в следующем месяце', 'через 2 месяца', 'в прошлом месяце', '2 месяца назад'];
        runFormatToPartsTest('ru', values, units[i], expected, options);

        expected = ['this month', 'next month', 'in 2 months', 'last month', '2 months ago'];
        runFormatToPartsTest('en', values, units[i], expected, options);

        expected = ['ce mois-ci', 'le mois prochain', 'dans 2 mois', 'le mois dernier', 'il y a 2 mois'];
        runFormatToPartsTest('fr', values, units[i], expected, options);
    }
}

function formatToPartsForQuarterTest() {
    const options: Intl.RelativeTimeFormatOptions = {numeric: 'auto', style: 'narrow', localeMatcher: 'best fit'};
    const units: FixedArray<string> = ['quarter', 'quarters'];
    const values: FixedArray<number> = [0, 1, 2, -1, -2];
    for (let i = 0; i < units.length; i++) {
        let expected: FixedArray<string> = ['本季度', '下季度', '2个季度后', '上季度', '2个季度前'];
        runFormatToPartsTest('zh', values, units[i], expected, options);

        expected = ['в текущем квартале', 'в следующем квартале', 'через 2 квартала', 'в прошлом квартале', '2 квартала назад'];
        runFormatToPartsTest('ru', values, units[i], expected, options);

        expected = ['this quarter', 'next quarter', 'in 2 quarters', 'last quarter', '2 quarters ago'];
        runFormatToPartsTest('en', values, units[i], expected, options);

        expected = ['ce trimestre', 'le trimestre prochain', 'dans 2 trimestres', 'le trimestre dernier', 'il y a 2 trimestres'];
        runFormatToPartsTest('fr', values, units[i], expected, options);
    }
}

function formatToPartsForYearTest() {
    const options: Intl.RelativeTimeFormatOptions = {numeric: 'auto', style: 'long', localeMatcher: 'lookup'};
    const units: FixedArray<string> = ['year', 'years'];
    const values: FixedArray<number> = [0, 1, 2, -1, -2];
    for (let i = 0; i < units.length; i++) {
        let expected: FixedArray<string> = ['今年', '明年', '2年后', '去年', '2年前'];
        runFormatToPartsTest('zh', values, units[i], expected, options);

        expected = ['в этом году', 'в следующем году', 'через 2 года', 'в прошлом году', '2 года назад'];
        runFormatToPartsTest('ru', values, units[i], expected, options);

        expected = ['this year', 'next year', 'in 2 years', 'last year', '2 years ago'];
        runFormatToPartsTest('en', values, units[i], expected, options);

        expected = ['cette année', 'l’année prochaine', 'dans 2 ans', 'l’année dernière', 'il y a 2 ans'];
        runFormatToPartsTest('fr', values, units[i], expected, options);
    }
}

function testFormatToParts() {
    formatToPartsForSencodTest();
    formatToPartsForMinuteTest();
    formatToPartsForHourTest();
    formatToPartsForDayTest();
    formatToPartsForWeekTest();
    formatToPartsForMonthTest();
    formatToPartsForQuarterTest();
    formatToPartsForYearTest();
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
