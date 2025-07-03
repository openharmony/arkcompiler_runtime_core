/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

const fakeLocale = "er-or"

function main() throws {
    testFormat()
    testFormatToParts()
    testSupportedLocalesOf()
}

function testFormat() {
    {
        const formatter = new Intl.ListFormat("en-US")
        const arr = new Array<string>()
        assertEQ(formatter.format(arr), "")
    }
    {
        const formatter = new Intl.ListFormat("en-US")
        const arr = Array.from(["apple"])
        assertEQ(formatter.format(arr), "apple")
    }
    {
        const formatter = new Intl.ListFormat("en-US", { style: "long", type: "conjunction" })
        const arr = Array.from(["apples", "oranges", "bananas"])
        assertEQ(formatter.format(arr), "apples, oranges, and bananas")
    }
    {
        const formatter = new Intl.ListFormat("ru-RU", { style: "long", type: "conjunction" })
        const arr = Array.from(["яблоки", "апельсины", "бананы"])
        assertEQ(formatter.format(arr), "яблоки, апельсины и бананы")
    }
    {
        const formatter = new Intl.ListFormat("en-US", { style: "short", type: "disjunction" })
        const arr = Array.from(["cats", "dogs", "birds"])
        assertEQ(formatter.format(arr), "cats, dogs, or birds")
    }
    {
        const formatter = new Intl.ListFormat("en-US", { style: "narrow", type: "unit" })
        const arr = Array.from(["5", "10", "15"])
        assertEQ(formatter.format(arr), "5 10 15")
    }
    {
        const formatter = new Intl.ListFormat("en-US")
        const arr = Array.from(["a", "b", "c"])
        assertEQ(formatter.format(arr), "a, b, and c")
    }
}

function testParts(parts: Array<Intl.FormatToPartsResult>, expected: Intl.FormatToPartsResult[]) {
    assertEQ(parts.length, expected.length)
    for (let i = 0; i < parts.length; i++) {
        assertEQ(parts[i].type, expected[i].type)
        assertEQ(parts[i].value, expected[i].value)
    }
}

function testFormatToParts() {
    {
        const formatter = new Intl.ListFormat("en-US")
        const arr = new Array<string>()
        testParts(
            formatter.formatToParts(arr),
            [] as Intl.FormatToPartsResult[]
        )
    }
    {
        const formatter = new Intl.ListFormat("en-US")
        const arr = Array.from(["apple"])
        testParts(
            formatter.formatToParts(arr),
            [
                { type: "element", value: "apple" }
            ] as Intl.FormatToPartsResult[]
        )
    }
    {
        const formatter = new Intl.ListFormat("en-US", { style: "long", type: "conjunction" })
        const arr = Array.from(["apples", "oranges", "bananas"])
        testParts(
            formatter.formatToParts(arr),
            [
                { type: "element", value: "apples" },
                { type: "literal", value: ", " },
                { type: "element", value: "oranges" },
                { type: "literal", value: ", and " },
                { type: "element", value: "bananas" },
            ] as Intl.FormatToPartsResult[]
        )
    }
    {
        const formatter = new Intl.ListFormat("ru-RU", { style: "long", type: "conjunction" })
        const arr = Array.from(["яблоки", "апельсины", "бананы"])
        testParts(
            formatter.formatToParts(arr),
            [
                { type: "element", value: "яблоки" },
                { type: "literal", value: ", " },
                { type: "element", value: "апельсины" },
                { type: "literal", value: " и " },
                { type: "element", value: "бананы" },
            ]
        )
    }
    {
        const formatter = new Intl.ListFormat("en-US", { style: "short", type: "disjunction" })
        const arr = Array.from(["cats", "dogs", "birds"])
        testParts(
            formatter.formatToParts(arr),
            [
                { type: "element", value: "cats" },
                { type: "literal", value: ", " },
                { type: "element", value: "dogs" },
                { type: "literal", value: ", or " },
                { type: "element", value: "birds" },
            ]
        )
    }
    {
        const formatter = new Intl.ListFormat("ru-RU", { style: "narrow", type: "unit" })
        const arr = Array.from(["5", "10", "15"])
        testParts(
            formatter.formatToParts(arr),
            [
                { type: "element", value: "5" },
                { type: "literal", value: " " },
                { type: "element", value: "10" },
                { type: "literal", value: " " },
                { type: "element", value: "15" },
            ]
        )
    }
    {
        const formatter = new Intl.ListFormat("en-US", { style: "long", type: "conjunction" })
        const arr = Array.from(["apples", "oranges"])
        testParts(
            formatter.formatToParts(arr),
            [
                { type: "element", value: "apples" },
                { type: "literal", value: " and " },
                { type: "element", value: "oranges" },
            ]
        )
    }
    {
        const formatter = new Intl.ListFormat("ru-RU")
        const arr = Array.from(["а", "б", "в"])
        testParts(
            formatter.formatToParts(arr),
            [
                { type: "element", value: "а" },
                { type: "literal", value: ", " },
                { type: "element", value: "б" },
                { type: "literal", value: " и " },
                { type: "element", value: "в" },
            ]
        )
    }
}

function runSupportedLocalesTest(
    locales: string | string[],
    options: Intl.ListFormatLocaleMatcher | undefined,
    assertEQed: string[])
{
    const result = Intl.ListFormat.supportedLocalesOf(locales, options)
    assertEQ(result.length, assertEQed.length)
    for (let i = 0; i < result.length; i++) {
        assertEQ(result[i], assertEQed[i])
    }
}

function testSupportedLocalesOf() {
    runSupportedLocalesTest(
        "en",
        undefined,
        intlBestFitLocales("en")
    )
    runSupportedLocalesTest(
        fakeLocale,
        undefined,
        intlBestFitLocales(fakeLocale)
    )
    runSupportedLocalesTest(
        ["fr", fakeLocale, "ru"],
        undefined,
        intlBestFitLocales(["fr", fakeLocale, "ru"])
    )
    runSupportedLocalesTest(
        ["ru-RU", "zh-CN", "en-US"],
        undefined,
        intlBestFitLocales(["ru-RU", "zh-CN", "en-US"])
    )
    runSupportedLocalesTest(
        ["ru-RU", "zh-CN", "en-US"],
        "best fit",
        intlBestFitLocales(["ru-RU", "zh-CN", "en-US"])
    )

    runSupportedLocalesTest(
        "en",
        "lookup",
        intlLookUpLocales("en")
    )
    runSupportedLocalesTest(
        fakeLocale,
        "lookup",
        intlLookUpLocales(fakeLocale)
    )

    runSupportedLocalesTest(
        ["ru-RU", "zh-CN", fakeLocale, "en-US"],
        "lookup",
        intlLookUpLocales(["ru-RU", "zh-CN", fakeLocale, "en-US"])
    )
}
