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
    suite.addTest("$_invoke", testInvoke);
    suite.addTest("$_invoke currency style error", testInvokeCurrencyStyleError);
    suite.addTest("$_invoke currency unit error", testInvokeCurrencyStyleError);
    suite.addTest("$_invoke undefined locale", testInvokeUndefinedLocale);
    suite.addTest("$_invoke undefined locale undefined options", testInvokeUndefinedLocaleUndefinedOptions);
    return suite.run();
}

function testInvoke() {
    let expected = new Intl.NumberFormat("en-EN");
    let result = Intl.NumberFormat.$_invoke("en-EN");
    let value = -123456.789;
    let isEqualed = equaledNumberFormats(result, expected, value);
    assertTrue(isEqualed, "Error. Got: " + result.format(value) + ". Expected: " + expected.format(value));
}

function testInvokeCurrencyStyleError() {
    let expectedError = new RangeError("Currency code is required with currency style");
    expectError((): void throws => { Intl.NumberFormat.$_invoke("en-EN", { style: "currency" } as Intl.NumberFormatOptions) }, expectedError);
}

function testInvokeUnitStyleError() {
    let expectedError = new RangeError("Unit code is required with unit style");
    expectError((): void throws => { Intl.NumberFormat.$_invoke("en-EN", { style: "unit" } as Intl.NumberFormatOptions) }, expectedError);
}

function testInvokeUndefinedLocale() {
    let numberFormat = new Intl.NumberFormat(undefined);
    assertTrue(numberFormat != undefined && numberFormat != null);
    let getOptions = numberFormat.resolvedOptions();
    let s: string = printResolvedOptions(getOptions);
    assertTrue(getOptions.locale == intlBestFitLocale(""), "Error. Got: " + s);
    assertTrue(getOptions.numberingSystem == "latn", "Error. Got: " + s);
    assertTrue(getOptions.style == "decimal", "Error. Got: " + s);
    assertTrue(getOptions.minimumIntegerDigits == 1, "Error. Got: " + s);
    assertTrue(getOptions.minimumFractionDigits == 0, "Error. Got: " + s);
    assertTrue(getOptions.maximumFractionDigits == 3, "Error. Got: " + s);
    assertTrue(getOptions.useGrouping == false, "Error. Got: " + s);
    assertTrue(getOptions.notation == "standard", "Error. Got: " + s);
    assertTrue(getOptions.signDisplay == "auto", "Error. Got: " + s);
}

function testInvokeUndefinedLocaleUndefinedOptions() {
    let numberFormat = new Intl.NumberFormat(undefined, undefined);
    assertTrue(numberFormat != undefined && numberFormat != null);
    let getOptions = numberFormat.resolvedOptions();
    let s: string = printResolvedOptions(getOptions);
    assertTrue(getOptions.locale == intlBestFitLocale(""), "Error. Got: " + s);
    assertTrue(getOptions.numberingSystem == "latn", "Error. Got: " + s);
    assertTrue(getOptions.style == "decimal", "Error. Got: " + s);
    assertTrue(getOptions.minimumIntegerDigits == 1, "Error. Got: " + s);
    assertTrue(getOptions.minimumFractionDigits == 0, "Error. Got: " + s);
    assertTrue(getOptions.maximumFractionDigits == 3, "Error. Got: " + s);
    assertTrue(getOptions.useGrouping == false, "Error. Got: " + s);
    assertTrue(getOptions.notation == "standard", "Error. Got: " + s);
    assertTrue(getOptions.signDisplay == "auto", "Error. Got: " + s);
}

function equaledNumberFormats(a: Intl.NumberFormat, b: Intl.NumberFormat, value: number | bigint): boolean {
    if (a.format(value) != b.format(value)) {
        return false;
    }
    return true;
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
