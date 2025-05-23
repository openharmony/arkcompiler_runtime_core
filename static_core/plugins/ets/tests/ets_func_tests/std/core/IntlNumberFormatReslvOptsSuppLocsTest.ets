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

function main(): int {
    let suite = new ArkTestsuite("Intl. NumberFormat");
    suite.addTest("resolvedOptions default", testResolvedOptions);
    suite.addTest("resolvedOptions decimal", testResolvedOptionsDecimal);
    suite.addTest("resolvedOptions percent", testResolvedOptionsPercent);
    suite.addTest("resolvedOptions currency", testResolvedOptionsCurrency);
    suite.addTest("supportedLocalesOf", testSupportedLocalesOf);
    suite.addTest("supportedLocalesOfEmpty", testSupportedLocalesOfEmpty);
    suite.addTest("supportedLocalesOfUndefined", testSupportedLocalesOfUndefined);
    return suite.run();
}

function testSupportedLocalesOf() {
    let en_US: string = intlBestFitLocale("en-US");
    let zh_CN: string = intlBestFitLocale("zh-CN");
    let ru_RU: string = intlBestFitLocale("ru-RU");
    let localeList: string[] = [ru_RU, zh_CN, en_US];
    let expected: string[] = [ru_RU, zh_CN, en_US];
    let result: string[] = Intl.NumberFormat.supportedLocalesOf(localeList);
    assertTrue(expected[0] == result[0], "Error. Got: " + result[0] + ". Expected: " + expected[0]);
    assertTrue(expected[1] == result[1], "Error. Got: " + result[1] + ". Expected: " + expected[1]);
    assertTrue(expected[2] == result[2], "Error. Got: " + result[2] + ". Expected: " + expected[2]);
}

function testSupportedLocalesOfEmpty() {
    let locales = ""
    let expectedError = new Error("Incorrect locale information provided");
    expectError((): void throws => { Intl.NumberFormat.supportedLocalesOf(locales); }, expectedError);
}

function testSupportedLocalesOfUndefined() {
    let locales = ""
    let expectedError = new Error("Incorrect locale information provided");
    expectError((): void throws => { Intl.NumberFormat.supportedLocalesOf(locales, undefined); }, expectedError);
}

function testResolvedOptions() {
    let expected: Intl.ResolvedNumberFormatOptions = {
            locale: "en",
            numberingSystem: "latn",
            style: "decimal",
            useGrouping: false,
            notation: "standard",
            signDisplay: "auto",
            maximumFractionDigits: 3,
            minimumFractionDigits: 0,
            minimumIntegerDigits: 1
        }
    let result = new Intl.NumberFormat("en", undefined).resolvedOptions();
    let isEqualed = equaledResolvedNumberFormatOptions(result, expected);
    assertTrue(isEqualed, "Error: Got: " + printResolvedOptions(result) + ".\n\nExpected: " + printResolvedOptions(expected));
    result = new Intl.NumberFormat("en").resolvedOptions();
    isEqualed = equaledResolvedNumberFormatOptions(result, expected);
    assertTrue(isEqualed);
}

function testResolvedOptionsDecimal() {
    let options: Intl.NumberFormatOptions = {
        style: "decimal"
    }
    let getOptions = new Intl.NumberFormat("de", options).resolvedOptions();
    let s: string = printResolvedOptions(getOptions);
    assertTrue(getOptions.locale == "de", "Error. Got: " + s);
    assertTrue(getOptions.numberingSystem == "latn", "Error. Got: " + s);
    assertTrue(getOptions.style == "decimal", "Error. Got: " + s);
    assertTrue(getOptions.minimumIntegerDigits == 1, "Error. Got: " + s);
    assertTrue(getOptions.minimumFractionDigits == 0, "Error. Got: " + s);
    assertTrue(getOptions.maximumFractionDigits == 3, "Error. Got: " + s);
    assertTrue(getOptions.useGrouping == false, "Error. Got: " + s);
    assertTrue(getOptions.notation == "standard", "Error. Got: " + s);
    assertTrue(getOptions.signDisplay == "auto", "Error. Got: " + s);
}

function testResolvedOptionsPercent() {
    let options: Intl.NumberFormatOptions = {
        style: "percent"
    }
    let getOptions = new Intl.NumberFormat("de", options).resolvedOptions();
    let s: string = printResolvedOptions(getOptions);
    assertTrue(getOptions.locale == "de", "Error. Got: " + s);
    assertTrue(getOptions.numberingSystem == "latn", "Error. Got: " + s);
    assertTrue(getOptions.style == "percent", "Error. Got: " + s);
    assertTrue(getOptions.minimumIntegerDigits == 1, "Error. Got: " + s);
    assertTrue(getOptions.minimumFractionDigits == 0, "Error. Got: " + s);
    assertTrue(getOptions.maximumFractionDigits == 0, "Error. Got: " + s);
    assertTrue(getOptions.useGrouping == true, "Error. Got: " + s)
    assertTrue(getOptions.notation == "standard", "Error. Got: " + s);
    assertTrue(getOptions.signDisplay == "auto", "Error. Got: " + s);
}

function testResolvedOptionsCurrency() {
    let options: Intl.NumberFormatOptions = {
        style: "currency", currency: "CNY"
    }
    let getOptions = new Intl.NumberFormat("zh", options).resolvedOptions();
    let s: string = printResolvedOptions(getOptions);
    assertTrue(getOptions.locale == "zh", "Error. Got: " + s);
    assertTrue(getOptions.numberingSystem == "latn", "Error. Got: " + s);
    assertTrue(getOptions.style == "currency", "Error. Got: " + s);
    assertTrue(getOptions.currency == "CNY", "Error. Got: " + s);
    assertTrue(getOptions.currencyDisplay == "symbol", "Error. Got: " + s);
    assertTrue(getOptions.currencySign == "standard", "Error. Got: " + s);
    assertTrue(getOptions.minimumIntegerDigits == 1, "Error. Got: " + s);
    assertTrue(getOptions.minimumFractionDigits == 2, "Error. Got: " + s);
    assertTrue(getOptions.maximumFractionDigits == 2, "Error. Got: " + s);
    assertTrue(getOptions.useGrouping == true, "Error. Got: " + s);
    assertTrue(getOptions.notation == "standard", "Error. Got: " + s);
    assertTrue(getOptions.signDisplay == "auto", "Error. Got: " + s);
}

function equaledResolvedNumberFormatOptions(a: Intl.ResolvedNumberFormatOptions, b: Intl.ResolvedNumberFormatOptions): boolean {
    return a.compactDisplay == b.compactDisplay
            && a.currencySign == b.currencySign
            && a.currency == b.currency
            && a.currencyDisplay == b.currencyDisplay
            && a.locale == b.locale
            && a.maximumFractionDigits == b.maximumFractionDigits
            && a.maximumSignificantDigits == b.maximumSignificantDigits
            && a.minimumFractionDigits == b.minimumFractionDigits
            && a.minimumIntegerDigits == b.minimumIntegerDigits
            && a.minimumSignificantDigits == b.minimumSignificantDigits
            && a.notation == b.notation
            && a.numberingSystem == b.numberingSystem
            && a.signDisplay == b.signDisplay
            && a.style == b.style
            && a.unit == b.unit
            && a.unitDisplay == b.unitDisplay
            && a.useGrouping == b.useGrouping;
}

function printResolvedOptions(options: Intl.ResolvedNumberFormatOptions): string {
    let parts = new Array<string>
    let addValue = (value: number|string|boolean|undefined, name: string) => {
        let s = name + " : " + (value === undefined ? "undefined" : String(value));
        parts.push(s);
    };
    addValue(options.compactDisplay, "compactDisplay");
    addValue(options.currencySign, "currencySign");
    addValue(options.currency, "currency");
    addValue(options.currencyDisplay, "currencyDisplay");
    addValue(options.locale, "locale");
    addValue(options.maximumFractionDigits, "maximumFractionDigits");
    addValue(options.minimumFractionDigits, "minimumFractionDigits");
    addValue(options.maximumSignificantDigits, "maximumSignificantDigits");
    addValue(options.minimumSignificantDigits, "minimumSignificantDigits");
    addValue(options.minimumIntegerDigits, "minimumIntegerDigits");
    addValue(options.notation, "notation");
    addValue(options.numberingSystem, "numberingSystem");
    addValue(options.signDisplay, "signDisplay");
    addValue(options.style, "style");
    addValue(options.unit, "unit");
    addValue(options.unitDisplay, "unitDisplay");
    addValue(options.useGrouping, "useGrouping");
    return parts.join(",");
}
