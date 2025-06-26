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
    testSelect()
    testResolvedOptions()
    testSelectWithOptions()
    testSupportedLocalesOf()
}

function runSelectTest(locale: string, value: number, expected: string, options?: Intl.PluralRulesOptions) {
    const rules = new Intl.PluralRules(locale, options)
    const result = rules.select(value)
    assertEQ(result, expected)
}

function testSelect() {
    runSelectTest("en", 1, "one")
    runSelectTest("en", 2, "other")
    runSelectTest("en", 0, "other")
  
    runSelectTest("ru", 1, "one")
    runSelectTest("ru", 2, "few")
    runSelectTest("ru", 5, "many")
    runSelectTest("ru", 21, "one")
    runSelectTest("ru", 102, "few")
    runSelectTest("ru", 1005, "many")
  
    runSelectTest("fr", 1, "one")
    runSelectTest("fr", 2, "other")
    runSelectTest("fr", 0, "one")
  
    runSelectTest("zh", 1, "other")
    runSelectTest("zh", 100, "other")
    runSelectTest("zh", 0, "other")
  
    runSelectTest("ja", 1, "other")
    runSelectTest("ja", 5, "other")
  
    runSelectTest(fakeLocale, 1, "one")
    runSelectTest(fakeLocale, 2, "other")
}

function testResolvedOptions() {
    {
        const rules = new Intl.PluralRules("en")
        const result = rules.resolvedOptions()
        assertEQ(result.minimumIntegerDigits, 1)
        assertEQ(result.minimumFractionDigits, 0)
        assertEQ(result.maximumFractionDigits, 3)
        assertEQ(result.minimumSignificantDigits, undefined)
        assertEQ(result.maximumSignificantDigits, undefined)
        assertEQ(result.type, "cardinal")
    }
    {
        const rules = new Intl.PluralRules("en", { type: "ordinal" } as Intl.PluralRulesOptions)
        const result = rules.resolvedOptions()
        assertEQ(result.type, "ordinal")
    }
    {
        const rules = new Intl.PluralRules("en", { minimumFractionDigits: 2 } as Intl.PluralRulesOptions)
        const result = rules.resolvedOptions()
        assertEQ(result.minimumFractionDigits, 2)
    }
    {
        const rules = new Intl.PluralRules("en", { maximumFractionDigits: 4 } as Intl.PluralRulesOptions)
        const result = rules.resolvedOptions()
        assertEQ(result.maximumFractionDigits, 4)
    }
    {
        const rules = new Intl.PluralRules("en", { minimumSignificantDigits: 3 } as Intl.PluralRulesOptions)
        const result = rules.resolvedOptions()
        assertEQ(result.minimumSignificantDigits, 3)
    }
}

function runTestWithOptions(locale: string, value: number, options: Intl.PluralRulesOptions, expected: string) {
    const rules = new Intl.PluralRules(locale, options)
    const result = rules.select(value)
    assertEQ(result, expected)
}

function testSelectWithOptions() {
    runTestWithOptions("en", 1, { type: "cardinal" }, "one")
    runTestWithOptions("en", 2, { type: "cardinal" }, "other")
    runTestWithOptions("en", 1, { type: "ordinal" }, "one")
    runTestWithOptions("en", 2, { type: "ordinal" }, "two")
    runTestWithOptions("en", 3, { type: "ordinal" }, "few")
    runTestWithOptions("en", 4, { type: "ordinal" }, "other")

    runTestWithOptions("fr", 1, { localeMatcher: "lookup" }, "one")
    runTestWithOptions("fr", 1, { localeMatcher: "best fit" }, "one")

    runTestWithOptions("fr", 0, { minimumFractionDigits: 2 }, "one")
    runTestWithOptions("fr", 0.5, { maximumFractionDigits: 1 }, "one")
    runTestWithOptions("fr", 1.5, { minimumFractionDigits: 2, maximumFractionDigits: 2 }, "one")
    runTestWithOptions("en", 1.99, { minimumFractionDigits: 1, maximumFractionDigits: 2 }, "other")
    runTestWithOptions("en", 2.5, { minimumFractionDigits: 1 }, "other")

    runTestWithOptions("de", 1, { minimumIntegerDigits: 2 }, "one")
    runTestWithOptions("de", 9, { minimumIntegerDigits: 3 }, "other")

    runTestWithOptions("ru", 1, { minimumSignificantDigits: 2 }, "other")
    runTestWithOptions("ru", 12, { minimumSignificantDigits: 2, maximumSignificantDigits: 3 }, "many")
    runTestWithOptions("ru", 1234, { minimumSignificantDigits: 3, maximumSignificantDigits: 5 }, "few")
    runTestWithOptions("ru", 0.0005, { minimumSignificantDigits: 1, maximumSignificantDigits: 2 }, "other")

    runTestWithOptions("ja", 0, {}, "other")
    runTestWithOptions("zh", 2, {}, "other")
    runTestWithOptions("ar", 0, {}, "zero")
    runTestWithOptions("ar", 1, {}, "one")
    runTestWithOptions("ar", 2, {}, "two")
    runTestWithOptions("ar", 3, {}, "few")
    runTestWithOptions("ar", 11, {}, "many")
    runTestWithOptions("ar", 100, {}, "other")

    runTestWithOptions(
        "fr", 
        1234.567, 
        { 
            localeMatcher: "best fit", 
            type: "cardinal", 
            minimumIntegerDigits: 3, 
            minimumFractionDigits: 2, 
            maximumFractionDigits: 4, 
            minimumSignificantDigits: 3, 
            maximumSignificantDigits: 5 
        }, 
        "other"
    )
    runTestWithOptions(
        "fr", 
        1.0, 
        { 
            localeMatcher: "lookup", 
            type: "cardinal", 
            minimumIntegerDigits: 1, 
            minimumFractionDigits: 0, 
            maximumFractionDigits: 2, 
            minimumSignificantDigits: 1, 
            maximumSignificantDigits: 3 
        }, 
        "one"
    )
    runTestWithOptions(
        "ar", 
        2, 
        { 
            localeMatcher: "best fit", 
            type: "cardinal", 
            minimumIntegerDigits: 1, 
            minimumFractionDigits: 0, 
            maximumFractionDigits: 3, 
            minimumSignificantDigits: 1, 
            maximumSignificantDigits: 5 
        }, 
        "two"
    )
    runTestWithOptions(
        "en", 
        3, 
        { 
            localeMatcher: "lookup", 
            type: "ordinal", 
            minimumIntegerDigits: 1, 
            minimumFractionDigits: 0, 
            maximumFractionDigits: 2, 
            minimumSignificantDigits: 1, 
            maximumSignificantDigits: 3 
        }, 
        "few"
    )
}

function runSupportedLocalesTest(
    locales: string | FixedArray<string>,
    options: Intl.SupportedLocalesOfOptions | undefined,
    expected: string[])
{
    const result = Intl.PluralRules.supportedLocalesOf(locales, options)
    assertEQ(result.length, expected.length)
    for (let i = 0; i < result.length; i++) {
        assertEQ(result[i], expected[i])
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
        { localeMatcher: "best fit" } as Intl.SupportedLocalesOfOptions,
        intlBestFitLocales(["ru-RU", "zh-CN", "en-US"])
    )

    runSupportedLocalesTest(
        "en",
        { localeMatcher: "lookup" } as Intl.SupportedLocalesOfOptions,
        intlLookUpLocales("en")
    )
    runSupportedLocalesTest(
        fakeLocale,
        { localeMatcher: "lookup" } as Intl.SupportedLocalesOfOptions,
        intlLookUpLocales(fakeLocale)
    )

    runSupportedLocalesTest(
        ["ru-RU", "zh-CN", fakeLocale, "en-US"],
        { localeMatcher: "lookup" } as Intl.SupportedLocalesOfOptions,
        intlLookUpLocales(["ru-RU", "zh-CN", fakeLocale, "en-US"])
    )
}
