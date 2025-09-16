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

const success: int = 0
const fail: int = 1

function main(): int {
    let failures: int = 0
    failures += check(testLocaleDefault(), "Test create default")
    failures += check(testParseTag(), "Test parse tag")
    failures += check(testHourCycle(), "Test hour cycle")
    failures += check(testCalendar(), "Test calendar")
    failures += check(testNumeric(), "Test numeric")
    failures += check(testCaseFirst(), "Test caseFirst")
    failures += check(testNumberingSystem(), "Test numbering system")
    failures += check(testCollation(), "Test collation")
    return failures
}

function check(result: int, message: string): int {
    if (result == 0) {
        return success
    }
    return fail
}

const errMsg = "Incorrect locale information provided"
function processCreateLocaleError(createLocaleFunction : () => Intl.Locale, msg ?: string) {
    let assertOccurred = false
    try {
        createLocaleFunction()
    }
    catch (e) {
        arktest.assertTrue(e instanceof RangeError)
        arktest.assertEQ((e as RangeError).message, (msg != undefined ? msg : errMsg))
        assertOccurred = true
    }
    finally {
        arktest.assertTrue(assertOccurred)
    }
}

function processInvalidValues(createLocaleFunction : ((arg : string) => Intl.Locale), msg ?: string) {
    const incorrectValues: ReadonlyArray<string> = Array.of<string>("", " ", "78", "Err", "!!!", "<<>>", "invalid")
    incorrectValues.forEach((invalidValue : string) => {
        processCreateLocaleError(() => createLocaleFunction(invalidValue), msg)
    })
}

function testLocaleDefault() : int {
    const l = new Intl.Locale("ru")
    if (l.language != null && l.language != undefined) {
        arktest.assertEQ(l.language!, "ru")
    }
    arktest.assertEQ(l.baseName, "ru")
    arktest.assertEQ(l.calendar, undefined)
    arktest.assertEQ(l.caseFirst, undefined)
    arktest.assertEQ(l.numeric, false)
    arktest.assertEQ(l.collation, undefined)
    arktest.assertEQ(l.hourCycle, undefined)
    arktest.assertEQ(l.numberingSystem, undefined)
    arktest.assertEQ(l.region, undefined)
    arktest.assertEQ(l.script, undefined)
    arktest.assertEQ(l.minimize().toString(), "ru")
    arktest.assertEQ(l.maximize().toString(), "ru-Cyrl-RU")
    return success
}

function testParseTag() : int {
    {
        const l = new Intl.Locale("ru-Cyrl")
        arktest.assertEQ(l.language, "ru")
        arktest.assertEQ(l.script, "Cyrl")
        arktest.assertEQ(l.region, undefined)
    }
    {
        const l = new Intl.Locale("ru-Ru")
        arktest.assertEQ(l.language, "ru")
        arktest.assertEQ(l.script, undefined)
        arktest.assertEQ(l.region, "RU")
    }
    {
        const ruRu = new Intl.Locale("ru-Ru-u-kn-false")
        arktest.assertEQ(ruRu.numeric, false)
    }
    {
        const ruRu = new Intl.Locale("ru-Ru-u-kn-true")
        arktest.assertEQ(ruRu.numeric, true)
    }
    {
        const ruRu = new Intl.Locale("ru-Ru-u-kn")
        arktest.assertEQ(ruRu.numeric, true)
    }
    {
        processCreateLocaleError(() => new Intl.Locale("ru-u-kn-fggkhjkjg", {"numeric" : false} as Intl.LocaleOptions))
    }
    {
        const ruRu = new Intl.Locale("ru-Ru-u-co-emoji")
        arktest.assertEQ(ruRu.collation, "emoji")
    }
    {
        const l = new Intl.Locale("ru-u-ca-gregory")
        arktest.assertEQ(l.calendar, "gregory")
    }
    {
        const l = new Intl.Locale("ru-u-hc-12h")
        arktest.assertEQ(l.hourCycle, "12h")
    }
    {
        const l = new Intl.Locale("ru-u-hc-invalid")
        arktest.assertEQ(l.hourCycle, "invalid")
    }
    {
        const l = new Intl.Locale("ru-u-kf-invalid")
        arktest.assertEQ(l.caseFirst, "invalid")
    }
    {
        const l = new Intl.Locale("ru-u-kf-lower")
        arktest.assertEQ(l.caseFirst, "lower")
    }
    {
        const l = new Intl.Locale("ru-u-nu-invalid")
        arktest.assertEQ(l.numberingSystem, "invalid")
    }
    {
        const l = new Intl.Locale("ru-u-nu-arab")
        arktest.assertEQ(l.numberingSystem, "arab")
    }
    return success
}

function testHourCycle() : int {
    {
        const us12hour = new Intl.Locale("en-US", { hourCycle: "h12" } as Intl.LocaleOptions)
        arktest.assertEQ(us12hour.hourCycle, "h12")
    }
    {
        processInvalidValues((invalidValue : string) => new Intl.Locale("en-US", { hourCycle: invalidValue } as Intl.LocaleOptions))
    }
    return success
}

function testCalendar() : int {
    {
        const loc = new Intl.Locale("en-US")
        arktest.assertEQ(loc.calendar, undefined)
    }
    {
        const loc = new Intl.Locale("en-US", {"calendar": "gregory"} as Intl.LocaleOptions)
        arktest.assertEQ(loc.calendar, "gregory")
    }
    {
        processInvalidValues((invalidValue : string) => new Intl.Locale("en-US", { calendar: invalidValue } as Intl.LocaleOptions))
    }
    return success
}

function testCaseFirst() : int {
    {
        const loc = new Intl.Locale("en-US")
        arktest.assertEQ(loc.caseFirst, undefined)
    }
    {
        const loc = new Intl.Locale("en-US", {"caseFirst": "upper"} as Intl.LocaleOptions)
        arktest.assertEQ(loc.caseFirst, "upper")
    }
    {
        processInvalidValues((invalidValue : string) => new Intl.Locale("ru-u-cf-invalid", {"caseFirst" : invalidValue} as Intl.LocaleOptions))
    }
    return success
}

function testNumeric() : int {
    {
        const l = new Intl.Locale("en-US", { numeric: true } as Intl.LocaleOptions)
        arktest.assertEQ(l.numeric as boolean, true)
    }
    {
        const l = new Intl.Locale("en-US", { numeric: false } as Intl.LocaleOptions)
        arktest.assertEQ(l.numeric as boolean, false)
    }
    {
        const l = new Intl.Locale("en-US")
        arktest.assertEQ(l.numeric as boolean, false)
    }

    return success
}

function testNumberingSystem() : int {
    {
        processInvalidValues((invalidValue : string) => new Intl.Locale("en-US", { numberingSystem: invalidValue } as Intl.LocaleOptions))
    }
    {
        const l = new Intl.Locale("en-US", { numberingSystem: "arab" } as Intl.LocaleOptions)
        arktest.assertEQ(l.numberingSystem as string, "arab")
    }
    {
        const l = new Intl.Locale("en-US")
        arktest.assertEQ(l.numberingSystem, undefined)
    }
    return success
}

function testCollation() : int {
    {
        const l = new Intl.Locale("en-US")
        arktest.assertEQ(l.collation, undefined)
    }
    {
        processInvalidValues((invalidValue : string) => new Intl.Locale("en-US", { collation: invalidValue } as Intl.LocaleOptions))
    }
    {
        const l = new Intl.Locale("ru", { collation: "emoji" } as Intl.LocaleOptions)
        arktest.assertEQ(l.collation, "emoji")
    }
    return success
}
