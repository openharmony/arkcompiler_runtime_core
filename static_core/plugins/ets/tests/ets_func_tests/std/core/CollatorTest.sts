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

const success: int = 0
const fail: int = 1
let en_US: string = intlBestFitLocale("en-US")
let zh_CN: string = intlBestFitLocale("zh-CN")
let ru_RU = intlBestFitLocale("ru-RU")
let er_ER = "er-or" // Fake locale
let localeList: string[] = [er_ER, ru_RU, zh_CN, en_US]
let correctLocaleList: string[] = [ru_RU, zh_CN, en_US]


function main(): int {
    let failures: int = 0
    failures += check(testSetOptions(), "Test set option values")
    failures += check(testOkLocalesConstructor(), "Test constructor with correct locales & default options")
    failures += check(testInvalidLocalesConstructor(), "Test constructor with invalid locales & default options")
    failures += check(testCompareStrings(), "Test compare strings")
    failures += check(testDefaultCollator(), "Test Collator with ru_RU locale & default options")
    failures += check(testSensivity(), "Test Collator with ru_RU locale & sensivity")
    failures += check(testPunctuation(), "Test Collator with en_US locale & punctuation")
    failures += check(testNumeric(), "Test Collator with ru_RU locale & numeric")
    failures += check(testCaseFirst(), "Test Collator with ru_RU locale & caseFirst")
    failures += check(testSupportedLocalesOf(), "Test static SupportedLocalesOf")
    failures += check(testUsage(), "Test Collator with search usage")
    failures += check(testCollation(), "Test Collator specific collation")
    
    return failures
}

function check(result: int, message: string): int {
    if (result == 0) {
        return success
    }
    console.println("\nFAILED: " + message)
    return fail
}

function testDefaultCollator() : int {
    const standardCollator = new Intl.Collator()
    if (standardCollator.resolvedOptions().locale == "ru-RU") {
        assertEQ(standardCollator.compare("Дом", "дом"), 1)
    }
    else {
        assertEQ(standardCollator.compare("House", "house"), 1)
    }
    let options = standardCollator.resolvedOptions()
    assertEQ(options.collation, "default")
    assertEQ(options.caseFirst, "false")
    assertEQ(options.ignorePunctuation, false)
    assertNE(standardCollator.compare("Hello, Geek! How are you?", "Hello Geek How are you?"), 0)
    return success
}

function testAccents(collator : Intl.Collator, expected : int) {
    assertEQ(collator.compare("á", "a"), expected); 
    let input = "Carácter is personality Caña Pesó había"
    let processed = "Caracter is personality Cana Peso habia"
    assertEQ(collator.compare(input, processed), expected)
    let input2 = "Caráááááácter is personality Caña Pesó había"
    let processed2 = "Caraaaaaacter is personality Cana Peso habia"
    assertEQ(collator.compare(input2, processed2), expected)
    let input3 = "CARAÁÁÁÁÁCTER IS PERSONALITY CANA PESO HABIA"
    let processed3 = "CARAAAAAACTER IS PERSONALITY CANA PESO HABIA"
    assertEQ(collator.compare(input3, processed3), expected)
    assertEQ(collator.compare("á","áu"), -1)
}

function containsRuLocale(collator : Intl.Collator) : boolean {
    return collator.resolvedOptions().locale.includes("ru") 
}

function testSensivity() : int {
    {
        const collator = new Intl.Collator(ru_RU, {sensitivity: "base"} as Intl.CollatorOptions)
        if (containsRuLocale(collator)) {
            assertEQ(collator.compare("Дом", "дом"), 0)
        }
        else {
            assertEQ(collator.compare("House", "house"), 0)
        }
        assertEQ(collator.compare("House", "house"), 0)
        assertNE(collator.compare("efw", "hgfcjg"), 0)
        testAccents(collator, 0)
    }
    {
        const collator = new Intl.Collator(ru_RU, {sensitivity: "accent"} as Intl.CollatorOptions)
        if (containsRuLocale(collator)) {
            assertEQ(collator.compare("Дом", "дом"), 0)
        }
        else {
            assertEQ(collator.compare("House", "house"), 0)
        }
        assertNE(collator.compare("efw", "hgfcjg"), 0)
        testAccents(collator, 1)
    }
    {
        const collator = new Intl.Collator(ru_RU, {sensitivity: "case"} as Intl.CollatorOptions)
        if (containsRuLocale(collator)) {
            assertEQ(collator.compare("Дом", "дом"), 1)
        }
        else {
            assertEQ(collator.compare("House", "house"), 1)
        }
        assertNE(collator.compare("efw", "hgfcjg"), 0)
        testAccents(collator, 0)
    }
    {
        const collator = new Intl.Collator(ru_RU, {sensitivity: "variant"} as Intl.CollatorOptions)
        if (containsRuLocale(collator)) {
            assertEQ(collator.compare("Дом", "дом"), 1)
        }
        else {
            assertEQ(collator.compare("House", "house"), 1)
        }
        assertNE(collator.compare("efw", "hgfcjg"), 0)
        testAccents(collator, 1)
    }
    
    return success
}

function testPunctuation() : int {
    {
        const collator = new Intl.Collator(en_US, {ignorePunctuation: true} as Intl.CollatorOptions)
        assertEQ(collator.compare("Hello, Geek! How are you?", "Hello Geek How are you?"), 0)
    }
    {
        const collator = new Intl.Collator(en_US, {ignorePunctuation: false} as Intl.CollatorOptions)
        assertNE(collator.compare("Hello, Geek! How are you?", "Hello Geek How are you?"), 0)
    }
    return success
}

function testNumeric() : int {
    const collator = new Intl.Collator(ru_RU, {sensitivity: "base", "numeric": true}  as Intl.CollatorOptions)
    assertEQ(collator.compare("3", "14"), -1)
    assertEQ(collator.compare("2", "15"), -1)
    assertEQ(collator.compare("9", "61"), -1)
    assertEQ(collator.compare("14", "3"), 1)
    assertEQ(collator.compare("14.98", "3.021"), 1)
    assertEQ(collator.compare("14,98", "3,021"), 1)

    // comparing 2 negative numbers - compares by abs
    assertEQ(collator.compare("-0.001", "-0.99"), -1)
    assertEQ(collator.compare("-1", "-2"), -1)
    assertEQ(collator.compare("-100", "-200"), -1)

    // strange collator logic with comparing numbers with differents signs
    assertEQ(collator.compare("0", "-1"), 1)
    assertEQ(collator.compare("5", "-5"), 1)
    assertEQ(collator.compare("5", "-4"), 1)
    assertEQ(collator.compare("5", "-200"), 1)

    assertEQ(collator.compare("-1", "0"), -1)
    assertEQ(collator.compare("-5", "5"), -1)
    assertEQ(collator.compare("-5", "4"), -1)
    assertEQ(collator.compare("-5", "200"), -1)

    assertEQ(collator.compare("4", "3"), 1)
    assertEQ(collator.compare("24", "31"), -1)

    assertEQ(collator.compare("-1", "6557899"), -1) //((-1 > 6557899) ? 1 : -1));
    assertEQ(collator.compare( "-6557899", "1"), -1) //((-6557899 > 1) ? 1 : -1));

    if (containsRuLocale(collator)) {
        assertEQ(collator.compare("Дом", "дом"), 0)
    }
    else {
        assertEQ(collator.compare("House", "house"), 0)
    }
    return success
}

function testCaseFirst() : int {
    {
        const collator = new Intl.Collator(ru_RU, {caseFirst: "upper"} as Intl.CollatorOptions)
        if (containsRuLocale(collator)) {
            assertEQ(collator.compare("Дом", "дом"), -1)
        }
        else {
            assertEQ(collator.compare("House", "house"), -1)
        }
    }
    {
        const collator = new Intl.Collator(ru_RU, {caseFirst: "false"} as Intl.CollatorOptions)
        if (containsRuLocale(collator)) {
            assertEQ(collator.compare("Дом", "дом"), 1)
        }
        else {
            assertEQ(collator.compare("House", "house"), 1)
        }
    }

    {
        const collator = new Intl.Collator(ru_RU, {caseFirst: "lower"} as Intl.CollatorOptions)
        if (containsRuLocale(collator)) {
            assertEQ(collator.compare("Дом", "дом"), 1)
            assertEQ(collator.compare("дОооом", "доооом"), 1)
            assertEQ(collator.compare("дщооом", "дЩооом"), -1)
            assertEQ(collator.compare("дооооМ", "доооом"), 1)
            assertEQ(collator.compare("доооом", "доооом"), 0)
        }
        else {
            assertEQ(collator.compare("House", "house"), 1)
            assertEQ(collator.compare("hOuse", "house"), 1)
            assertEQ(collator.compare("hxouse", "hXouse"), -1)
            assertEQ(collator.compare("housE", "house"), 1)
            assertEQ(collator.compare("house", "house"), 0)
        }
    }

    return success
}

function testOkLocalesConstructor() : int {
    let locales = [ru_RU, zh_CN, en_US]
    let c = new Intl.Collator(locales)
    return success
}

function testInvalidLocalesConstructor() : int {
    let c = new Intl.Collator(localeList);
    // TODO Incorrect locale information provided 
    const collator = new Intl.Collator("Err-er")
    return success
}

function testCompareStrings() : int {
    let collator = new Intl.Collator();
    let result = collator.compare("test0", "test1")
    assertNE(result, 0)

    result = collator.compare("test0", "test0")
    assertEQ(result, 0)

    return success
}

function testSupportedLocalesOf() : int  {
    let supportedLocales : string[] = Intl.Collator.supportedLocalesOf(localeList)
    assertTrue(supportedLocales.toString().includes(ru_RU))

    return success
}

function testSetOptions()
{
    {
        const collator = new Intl.Collator(en_US, {ignorePunctuation: true} as Intl.CollatorOptions)
        assertTrue(collator.resolvedOptions().ignorePunctuation)
        assertFalse(collator.resolvedOptions().numeric)
        assertEQ(collator.resolvedOptions().caseFirst, "false")
        assertEQ(collator.resolvedOptions().sensitivity, "variant")
        assertEQ(collator.resolvedOptions().usage, "sort")
    }
    {
        const collator = new Intl.Collator(en_US, {numeric: true} as Intl.CollatorOptions)
        assertFalse(collator.resolvedOptions().ignorePunctuation)
        assertTrue(collator.resolvedOptions().numeric)
        assertEQ(collator.resolvedOptions().caseFirst, "false")
        assertEQ(collator.resolvedOptions().sensitivity, "variant")
        assertEQ(collator.resolvedOptions().usage, "sort")
    }
    {
        const collator = new Intl.Collator(en_US, {caseFirst: "upper"} as Intl.CollatorOptions)
        assertFalse(collator.resolvedOptions().ignorePunctuation)
        assertFalse(collator.resolvedOptions().numeric)
        assertEQ(collator.resolvedOptions().caseFirst, "upper")
        assertEQ(collator.resolvedOptions().sensitivity, "variant")
        assertEQ(collator.resolvedOptions().usage, "sort")
    }
    {
        const collator = new Intl.Collator(en_US, {sensitivity: "accent"} as Intl.CollatorOptions)
        assertFalse(collator.resolvedOptions().ignorePunctuation)
        assertFalse(collator.resolvedOptions().numeric)
        assertEQ(collator.resolvedOptions().caseFirst, "false")
        assertEQ(collator.resolvedOptions().sensitivity, "accent")
        assertEQ(collator.resolvedOptions().usage, "sort")
    }
    {
        const collator = new Intl.Collator(en_US, {usage: "search"} as Intl.CollatorOptions)
        assertFalse(collator.resolvedOptions().ignorePunctuation)
        assertFalse(collator.resolvedOptions().numeric)
        assertEQ(collator.resolvedOptions().caseFirst, "false")
        assertEQ(collator.resolvedOptions().sensitivity, "variant")
        assertEQ(collator.resolvedOptions().usage, "search")
    }
    return success
}

// if sort - compare returns -1, 1 or 0; if search - only 0 or 1 except numeric compare
function testUsage() : int {
    {
        let options = {usage: "sort", caseFirst: "lower"} as Intl.CollatorOptions
        {
            const collator = new Intl.Collator(en_US, options)
            if (containsRuLocale(collator)) {
                assertEQ(collator.compare("дщооом", "дЩооом"), -1)
            }
            else {
                assertEQ(collator.compare("hxouse", "hXouse"), -1)
            }
        }
        {
            options.usage = "search"
            const collator = new Intl.Collator(en_US, options)
            if (containsRuLocale(collator)) {
                assertEQ(collator.compare("дщооом", "дЩооом"), 1)
            }
            else {
                assertEQ(collator.compare("hxouse", "hXouse"), 1)
            }
        }
    }
    {
        let options = {sensitivity: "base", "numeric": true}  as Intl.CollatorOptions
        {
            const collator = new Intl.Collator(ru_RU, options)
            assertEQ(collator.compare("3", "14"), -1);
        }
        {
            options.usage = "search"
            const collator = new Intl.Collator(ru_RU, options)
            assertEQ(collator.compare("3", "14"), -1);
        }
    }
    {
        let options = {sensitivity: "case"} as Intl.CollatorOptions
        {
            const collator = new Intl.Collator(ru_RU, options)
            if (containsRuLocale(collator)) {
                assertEQ(collator.compare("дом", "Дом"), -1)
            }
            else {
                assertEQ(collator.compare("house", "House"), -1)
            }
        }
        {
            options.usage = "search"
            const collator = new Intl.Collator(ru_RU, options)
            if (containsRuLocale(collator)) {
                assertEQ(collator.compare("дом", "Дом"), 1)
            }
            else {
                assertEQ(collator.compare("house", "House"), 1)
            }
        }
    }
    return success
}

function testCollation() : int {
    {
        // https://snipp.ru/handbk/emoji
        let c = new Intl.Collator(ru_RU, {collation: "emoji"} as Intl.CollatorOptions)
        assertEQ(c.compare("😀","😄"), -1)
        assertEQ(c.compare("😀","😀"), 0)
        assertEQ(c.compare("&#128512;", "&#128516;"), -1)
        assertEQ(c.compare("&#129315;", "&#128516;"), 1)
    }
    {
        // https://en.wikipedia.org/wiki/Pinyin_table
        let c = new Intl.Collator(ru_RU, {collation: "pinyin"} as Intl.CollatorOptions)
        assertEQ(c.compare( "扽", "ㄌㄨㄢˊ"), 1)
        assertEQ(c.compare( "方言", "方"), 1)
        assertEQ(c.compare("ㄖㄨㄚˊ", "挼"), -1)
    }

    return success
}

