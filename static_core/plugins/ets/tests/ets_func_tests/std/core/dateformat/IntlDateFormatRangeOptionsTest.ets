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

type FormatResult = [string | undefined, Intl.DateTimeFormatOptions | undefined, string]

const FORMAT_RESULTS = new Array<FormatResult>()

function addFormatResult(locale: string | undefined, options: Intl.DateTimeFormatOptions | undefined, result: string): void {
    const res: [string | undefined, Intl.DateTimeFormatOptions | undefined, string] = [locale, options, result]
    FORMAT_RESULTS.push(res)
}

const TEST_RANGE_BEGIN = new Date("2024-10-11T10:20:30Z")
const TEST_RANGE_END = new Date("2024-10-12T07:35:50Z")

function initFormatResults(): void {
    if (FORMAT_RESULTS.length > 0) {
        return
    }

    addFormatResult("en-US", { year: "numeric", month: "2-digit", day: "2-digit", timeZone: "UTC" }, "10/11/2024 – 10/12/2024")
}

function dateFormatRangeUsingFormattingOptionsTest(): void {
    FORMAT_RESULTS.forEach((res: FormatResult) => {
        const locale = res[0]
        const options = res[1]

        const formatter = new Intl.DateTimeFormat(locale, options)
        const actualResult = formatter.formatRange(TEST_RANGE_BEGIN, TEST_RANGE_END)

        const expectedResult = res[2]
        if (actualResult !== expectedResult) {
         arktest.assertEQ(actualResult, expectedResult, `Unexpected date formatting result for locale: ${locale} and options: ${options}`)
        }
    })
}

function main(): int {
    initFormatResults()

    const suite = new arktest.ArkTestsuite("Intl.DateTimeFormat.formatRange()")
    suite.addTest("using formatting options", dateFormatRangeUsingFormattingOptionsTest)
    return suite.run()
}
