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

const FORMAT_RESULTS = new Array<[Date, string | undefined, Intl.DateTimeFormatOptions | undefined, string]>()
function addDateFormatResult(date: Date, locale: string | undefined, options: Intl.DateTimeFormatOptions | undefined, result: string): void {
    const formatResult: [Date, string | undefined, Intl.DateTimeFormatOptions | undefined, string] = [date, locale, options, result]
    FORMAT_RESULTS.push(formatResult)
}

const UTC = "UTC"

function initFormatResults(): void {
    if (FORMAT_RESULTS.length > 0) {
        return
    }

    const testDate = new Date("2023-10-05T15:10:20.300Z")

    const detailedOptions: Intl.DateTimeFormatOptions = {year: "numeric", month: "long", day: "numeric", hour: "2-digit", minute: "2-digit", timeZone: UTC}
    addDateFormatResult(testDate, "de-DE", detailedOptions, "5. Oktober 2023 um 15:10")
    addDateFormatResult(testDate, "en-LR", detailedOptions, "5 October 2023 at 03:10 pm")

    addDateFormatResult(testDate, "de-DE", {year: "2-digit", month: "short", day: "numeric"}, "5. Okt. 23")
    addDateFormatResult(testDate, "de-DE", {year: "2-digit", month: "narrow", day: "numeric"}, "5. O 23")
    addDateFormatResult(testDate, "de-DE", {year: "2-digit", month: "2-digit", day: "numeric"}, "5.10.23")

    addDateFormatResult(testDate, "de-DE", {year: "2-digit", month: "2-digit", day: "2-digit", hour: "2-digit", minute: "2-digit", timeZone: UTC}, "05.10.23, 15:10")
}

function dateFormatWithLocaleAndOptions(): void {
    FORMAT_RESULTS.forEach(
        (formatResult: [Date, string | undefined, Intl.DateTimeFormatOptions | undefined, string], index: int) => {

        const locale = formatResult[1]
        const options = formatResult[2]

        const dateFormat = new Intl.DateTimeFormat(locale, options)

        const date = formatResult[0]
        const formattedDate = formatResult[3]

        arktest.assertEQ(formattedDate, dateFormat.format(date), `failed result index: ${index}`)
    })
}

function dateFormatWithDefaultLocale(): void {
    const defaultLocaleFormat = new Intl.DateTimeFormat()
    const date = new Date("2020-03-03")
    
    const dateStr = defaultLocaleFormat.format(date)

    // system locale dependent, so just checking that something returned
    arktest.assertTrue(dateStr != null)

    const undefLocaleFormat = new Intl.DateTimeFormat(undefined)
    arktest.assertEQ(dateStr, undefLocaleFormat.format(date))
}

function dateFormatWithCustomLocaleAndDefaultOptions(): void {
    const undefOptionsFormat = new Intl.DateTimeFormat("ru-RU", undefined)

    arktest.assertEQ(undefOptionsFormat.format(new Date("2021-04-04")), "04.04.2021")
}

function dateFormatWithInvalidTimeZone(): void {
    const options: Intl.DateTimeFormatOptions = { hour: "2-digit", minute: "2-digit", second: "2-digit", timeZone: "invalid" }
    const dateFormat = new Intl.DateTimeFormat("ru-RU", options)

    arktest.expectError(() => {
        dateFormat.format(new Date("2021-04-04T20:30:45Z"))
    }, "Invalid time zone specified: invalid")
}

function dateFormatWithoutParameters(): void {
    const options: Intl.DateTimeFormatOptions = { year: "numeric", month: "2-digit", day: "2-digit" }
    const formatter = new Intl.DateTimeFormat("en-US", options)

    const expected = formatter.format(Date.now())
    const actual = formatter.format()
    arktest.assertEQ(expected, actual)
}

function testConsoleLogNegativeDate() {
    let d = new Date(-271822)
    const MAX_CONSOLE_PRINT_DEPTH: int = 10
    // console.log stub
    let res = reflect.Value.of(d as Any).toPrint(MAX_CONSOLE_PRINT_DEPTH)
    arktest.assertEQ(res, "Date: \"1969-12-31T23:55:28.178Z\"")
}

function testDateForamtTest() {
    const date = new Date('2024-06-15T12:00:00Z')
    const formater = new Intl.DateTimeFormat('en-US', {
        timeZone: 'America/New_York',
        timeZoneName: 'long'
    });
    let a = formater.format(date)
    arktest.assertEQ(a, '6/15/2024, Eastern Daylight Time')

    const formatter2 = new Intl.DateTimeFormat('en-US', {
        timeZone: 'Asia/Tokyo',
        timeZoneName: 'long'
    });
    let result2 = formatter2.format(date);
    arktest.assertEQ(result2, '6/15/2024, Japan Standard Time');

    const formatter3 = new Intl.DateTimeFormat('en-US', {
        timeZone: 'America/Los_Angeles',
        timeZoneName: 'long'
    });
    let result3 = formatter3.format(date);
    arktest.assertEQ(result3, '6/15/2024, Pacific Daylight Time');

    const date1 = new Date('2023-01-15T20:30:00Z');
    const formatter4 = new Intl.DateTimeFormat('en-US', {
        timeZone: 'America/Los_Angeles',
        timeZoneName: 'long'
    });
    let result4 = formatter4.format(date1);
    arktest.assertEQ(result4, '1/15/2023, Pacific Standard Time');
    
    // Test with different time and Europe/London timezone
    const date2 = new Date('2024-12-25T05:45:30Z');
    const formatter5 = new Intl.DateTimeFormat('en-US', {
        timeZone: 'Europe/London',
        timeZoneName: 'long'
    });
    let result5 = formatter5.format(date2);
    arktest.assertEQ(result5, '12/25/2024, Greenwich Mean Time');
    
    // Additional test with Asia/Tokyo timezone
    const date3 = new Date('2023-07-04T10:15:00Z');
    const formatter6 = new Intl.DateTimeFormat('en-US', {
        timeZone: 'Asia/Tokyo',
        timeZoneName: 'long'
    });
    let result6 = formatter6.format(date3);
    arktest.assertEQ(result6, '7/4/2023, Japan Standard Time');

    const date4 = new Date('2023-07-04T10:15:00Z');
    const formatter7 = new Intl.DateTimeFormat('en-US', {
        timeZone: 'Asia/Shanghai',
        timeZoneName: 'long'
    });
    let result7 = formatter7.format(date4);
    arktest.assertEQ(result7, '7/4/2023, China Standard Time');

    const date5 = new Date('2024-06-15T12:00:00Z');
    const formatter8 = new Intl.DateTimeFormat('en-US', {
        timeZone: 'Asia/Shanghai',
        timeZoneName: 'long'
    });
    let result8 = formatter8.format(date5);
    arktest.assertEQ(result8, '6/15/2024, China Standard Time');
}

function main(): int {
    initFormatResults()

    const suite = new arktest.ArkTestsuite("Intl.DateTimeFormat API date + time format tests")

    suite.addTest("date format with custom locale and options", dateFormatWithLocaleAndOptions)
    suite.addTest("date format with default locale", dateFormatWithDefaultLocale)
    suite.addTest("date format with custom locale and default options", dateFormatWithCustomLocaleAndDefaultOptions)
    suite.addTest("date format with custom locale and options containing invalid time zone", dateFormatWithInvalidTimeZone)
    suite.addTest("format w/o parameters", dateFormatWithoutParameters)
    suite.addTest("Test console.log out for Date with negative constructor", testConsoleLogNegativeDate)
    suite.addTest("date format test", testDateForamtTest)

    return suite.run()
}

