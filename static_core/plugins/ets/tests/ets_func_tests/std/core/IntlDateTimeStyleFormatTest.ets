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

function initFormatResults(): void {
    if (FORMAT_RESULTS.length > 0) {
        return
    }

    addFormatResult("en-US", {dateStyle: "full"}, "Wednesday, October 11, 2023")
    addFormatResult("en-US", {dateStyle: "full", timeStyle: "full", timeZone: "UTC"}, "Wednesday, October 11, 2023, 12:20:00 AM Coordinated Universal Time")
    addFormatResult("en-US", {dateStyle: "full", timeStyle: "long", timeZone: "UTC"}, "Wednesday, October 11, 2023, 12:20:00 AM UTC")
    addFormatResult("en-US", {dateStyle: "full", timeStyle: "medium", timeZone: "UTC"}, "Wednesday, October 11, 2023, 12:20:00 AM")
    addFormatResult("en-US", {dateStyle: "full", timeStyle: "short", timeZone: "UTC"}, "Wednesday, October 11, 2023, 12:20 AM")
    addFormatResult("en-US", {dateStyle: "long"}, "October 11, 2023")
    addFormatResult("en-US", {dateStyle: "long", timeStyle: "full", timeZone: "UTC"}, "October 11, 2023, 12:20:00 AM Coordinated Universal Time")
    addFormatResult("en-US", {dateStyle: "long", timeStyle: "long", timeZone: "UTC"}, "October 11, 2023, 12:20:00 AM UTC")
    addFormatResult("en-US", {dateStyle: "long", timeStyle: "medium", timeZone: "UTC"}, "October 11, 2023, 12:20:00 AM")
    addFormatResult("en-US", {dateStyle: "long", timeStyle: "short", timeZone: "UTC"}, "October 11, 2023, 12:20 AM")
    addFormatResult("en-US", {dateStyle: "medium"}, "Oct 11, 2023")
    addFormatResult("en-US", {dateStyle: "medium", timeStyle: "full", timeZone: "UTC"}, "Oct 11, 2023, 12:20:00 AM Coordinated Universal Time")
    addFormatResult("en-US", {dateStyle: "medium", timeStyle: "long", timeZone: "UTC"}, "Oct 11, 2023, 12:20:00 AM UTC")
    addFormatResult("en-US", {dateStyle: "medium", timeStyle: "medium", timeZone: "UTC"}, "Oct 11, 2023, 12:20:00 AM")
    addFormatResult("en-US", {dateStyle: "medium", timeStyle: "short", timeZone: "UTC"}, "Oct 11, 2023, 12:20 AM")
    addFormatResult("en-US", {dateStyle: "short"}, "10/11/23")
    addFormatResult("en-US", {dateStyle: "short", timeStyle: "full", timeZone: "UTC"}, "10/11/23, 12:20:00 AM Coordinated Universal Time")
    addFormatResult("en-US", {dateStyle: "short", timeStyle: "long", timeZone: "UTC"}, "10/11/23, 12:20:00 AM UTC")
    addFormatResult("en-US", {dateStyle: "short", timeStyle: "medium", timeZone: "UTC"}, "10/11/23, 12:20:00 AM")
    addFormatResult("en-US", {dateStyle: "short", timeStyle: "short", timeZone: "UTC"}, "10/11/23, 12:20 AM")
    addFormatResult("zh-CN", {dateStyle: "full"}, "2023年10月11日星期三")
    addFormatResult("zh-CN", {dateStyle: "full", timeStyle: "full", timeZone: "UTC"}, "2023年10月11日星期三 协调世界时 上午12:20:00")
    addFormatResult("zh-CN", {dateStyle: "full", timeStyle: "long", timeZone: "UTC"}, "2023年10月11日星期三 UTC 上午12:20:00")
    addFormatResult("zh-CN", {dateStyle: "full", timeStyle: "medium", timeZone: "UTC"}, "2023年10月11日星期三 上午12:20:00")
    addFormatResult("zh-CN", {dateStyle: "full", timeStyle: "short", timeZone: "UTC"}, "2023年10月11日星期三 上午12:20")
    addFormatResult("zh-CN", {dateStyle: "long"}, "2023年10月11日")
    addFormatResult("zh-CN", {dateStyle: "long", timeStyle: "full", timeZone: "UTC"}, "2023年10月11日 协调世界时 上午12:20:00")
    addFormatResult("zh-CN", {dateStyle: "long", timeStyle: "long", timeZone: "UTC"}, "2023年10月11日 UTC 上午12:20:00")
    addFormatResult("zh-CN", {dateStyle: "long", timeStyle: "medium", timeZone: "UTC"}, "2023年10月11日 上午12:20:00")
    addFormatResult("zh-CN", {dateStyle: "long", timeStyle: "short", timeZone: "UTC"}, "2023年10月11日 上午12:20")
    addFormatResult("zh-CN", {dateStyle: "medium"}, "2023年10月11日")
    addFormatResult("zh-CN", {dateStyle: "medium", timeStyle: "full", timeZone: "UTC"}, "2023年10月11日 协调世界时 上午12:20:00")
    addFormatResult("zh-CN", {dateStyle: "medium", timeStyle: "long", timeZone: "UTC"}, "2023年10月11日 UTC 上午12:20:00")
    addFormatResult("zh-CN", {dateStyle: "medium", timeStyle: "medium", timeZone: "UTC"}, "2023年10月11日 上午12:20:00")
    addFormatResult("zh-CN", {dateStyle: "medium", timeStyle: "short", timeZone: "UTC"}, "2023年10月11日 上午12:20")
    addFormatResult("zh-CN", {dateStyle: "short"}, "2023/10/11")
    addFormatResult("zh-CN", {dateStyle: "short", timeStyle: "full", timeZone: "UTC"}, "2023/10/11 协调世界时 上午12:20:00")
    addFormatResult("zh-CN", {dateStyle: "short", timeStyle: "long", timeZone: "UTC"}, "2023/10/11 UTC 上午12:20:00")
    addFormatResult("zh-CN", {dateStyle: "short", timeStyle: "medium", timeZone: "UTC"}, "2023/10/11 上午12:20:00")
    addFormatResult("zh-CN", {dateStyle: "short", timeStyle: "short", timeZone: "UTC"}, "2023/10/11 上午12:20")
}

const TEST_DATE = new Date("2023-10-11T00:20:00Z")

function optionsToString(options: Intl.DateTimeFormatOptions | undefined): string {
    if (options === undefined) {
        return "undefined"
    }

    const optionsStr = new StringBuilder()
    
    if (options.dateStyle) {
        optionsStr.append(`dateStyle: "${options.dateStyle}", `)
    }
    if (options.timeStyle) {
        optionsStr.append(`timeStyle: "${options.timeStyle}", `)
    }
    if (options.timeZone) {
        optionsStr.append(`timeZone: "${options.timeZone}", `)
    }
    
    const result = optionsStr.toString()
    if (result.length > 0) {
        return "{" + result.substring(0, result.length - 2) + "}"
    } else {
        return "{}"
    }
}

function dateFormatWithFormattingStyle(): void {
    FORMAT_RESULTS.forEach((res: FormatResult) => {
        const locale = res[0]
        const options = res[1]
        const formatter = new Intl.DateTimeFormat(locale, options)
        
        const expected = res[2]
        const formattedDate = formatter.format(TEST_DATE)

        const optionsStr = optionsToString(options)
        assertEQ(formattedDate, expected, `unexpected date format result for locale: ${locale}, options: ${optionsStr}`)
    })
}

function main(): int {
    initFormatResults()

    const suite = new ArkTestsuite("Intl.DateTimeFormat formatting style tests") 
    suite.addTest("date style", dateFormatWithFormattingStyle)
    return suite.run()
}

