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
        assertTrue(e instanceof RangeError)
        assertEQ((e as RangeError).message, (msg != undefined ? msg : errMsg))
        assertOccurred = true
    }
    finally {
        assertTrue(assertOccurred)
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
        assertEQ(l.language!, "ru")
    }
    assertEQ(l.baseName, "ru")
    assertEQ(l.calendar, undefined)
    assertEQ(l.caseFirst, undefined)
    assertEQ(l.numeric, false)
    assertEQ(l.collation, undefined)
    assertEQ(l.hourCycle, undefined)
    assertEQ(l.numberingSystem, undefined)
    assertEQ(l.region, undefined)
    assertEQ(l.script, undefined)
    assertEQ(l.minimize().toString(), "ru")
    assertEQ(l.maximize().toString(), "ru-Cyrl-RU")
    return success
}

function testParseTag() : int {
    {
        const l = new Intl.Locale("ru-Cyrl")
        assertEQ(l.language, "ru")
        assertEQ(l.script, "Cyrl")
        assertEQ(l.region, undefined)
    }
    {
        const l = new Intl.Locale("ru-Ru")
        assertEQ(l.language, "ru")
        assertEQ(l.script, undefined)
        assertEQ(l.region, "RU")
    }
    {
        const ruRu = new Intl.Locale("ru-Ru-u-kn-false")
        assertEQ(ruRu.numeric, false)
    }
    {
        const ruRu = new Intl.Locale("ru-Ru-u-kn-true")
        assertEQ(ruRu.numeric, true)
    }
    {
        const ruRu = new Intl.Locale("ru-Ru-u-kn")
        assertEQ(ruRu.numeric, true)
    }
    {
        processCreateLocaleError(() => new Intl.Locale("ru-u-kn-fggkhjkjg", {"numeric" : false} as Intl.LocaleOptions))
    }
    {
        const ruRu = new Intl.Locale("ru-Ru-u-co-emoji")
        assertEQ(ruRu.collation, "emoji")
    }
    {
        const l = new Intl.Locale("ru-u-ca-gregory")
        assertEQ(l.calendar, "gregory")
    }
    {
        const l = new Intl.Locale("ru-u-hc-12h")
        assertEQ(l.hourCycle, "12h")
    }
    {
        const l = new Intl.Locale("ru-u-hc-invalid")
        assertEQ(l.hourCycle, "invalid")
    }
    {
        const l = new Intl.Locale("ru-u-kf-invalid")
        assertEQ(l.caseFirst, "invalid")
    }
    {
        const l = new Intl.Locale("ru-u-kf-lower")
        assertEQ(l.caseFirst, "lower")
    }
    {
        const l = new Intl.Locale("ru-u-nu-invalid")
        assertEQ(l.numberingSystem, "invalid")
    }
    {
        const l = new Intl.Locale("ru-u-nu-arab")
        assertEQ(l.numberingSystem, "arab")
    }
    return success
}

function testHourCycle() : int {
    {
        const us12hour = new Intl.Locale("en-US", { hourCycle: "h12" } as Intl.LocaleOptions)
        assertEQ(us12hour.hourCycle, "h12")
    }
    {
        processInvalidValues((invalidValue : string) => new Intl.Locale("en-US", { hourCycle: invalidValue } as Intl.LocaleOptions))
    }
    return success
}

function testCalendar() : int {
    {
        const loc = new Intl.Locale("en-US")
        assertEQ(loc.calendar, undefined)
    }
    {
        const loc = new Intl.Locale("en-US", {"calendar": "gregory"} as Intl.LocaleOptions)
        assertEQ(loc.calendar, "gregory")
    }
    {
        processInvalidValues((invalidValue : string) => new Intl.Locale("en-US", { calendar: invalidValue } as Intl.LocaleOptions))
    }
    return success
}

function testCaseFirst() : int {
    {
        const loc = new Intl.Locale("en-US")
        assertEQ(loc.caseFirst, undefined)
    }
    {
        const loc = new Intl.Locale("en-US", {"caseFirst": "upper"} as Intl.LocaleOptions)
        assertEQ(loc.caseFirst, "upper")
    }
    {
        processInvalidValues((invalidValue : string) => new Intl.Locale("ru-u-cf-invalid", {"caseFirst" : invalidValue} as Intl.LocaleOptions))
    }
    return success
}

function testNumeric() : int {
    {
        const l = new Intl.Locale("en-US", { numeric: true } as Intl.LocaleOptions)
        assertEQ(l.numeric as boolean, true)
    }
    {
        const l = new Intl.Locale("en-US", { numeric: false } as Intl.LocaleOptions)
        assertEQ(l.numeric as boolean, false)
    }
    {
        const l = new Intl.Locale("en-US")
        assertEQ(l.numeric as boolean, false)
    }

    return success
}

function testNumberingSystem() : int {
    {
        processInvalidValues((invalidValue : string) => new Intl.Locale("en-US", { numberingSystem: invalidValue } as Intl.LocaleOptions))
    }
    {
        const l = new Intl.Locale("en-US", { numberingSystem: "arab" } as Intl.LocaleOptions)
        assertEQ(l.numberingSystem as string, "arab")
    }
    {
        const l = new Intl.Locale("en-US")
        assertEQ(l.numberingSystem, undefined)
    }
    return success
}

function testCollation() : int {
    {
        const l = new Intl.Locale("en-US")
        assertEQ(l.collation, undefined)
    }
    {
        processInvalidValues((invalidValue : string) => new Intl.Locale("en-US", { collation: invalidValue } as Intl.LocaleOptions))
    }
    {
        const l = new Intl.Locale("ru", { collation: "emoji" } as Intl.LocaleOptions)
        assertEQ(l.collation, "emoji")
    }
    return success
}
