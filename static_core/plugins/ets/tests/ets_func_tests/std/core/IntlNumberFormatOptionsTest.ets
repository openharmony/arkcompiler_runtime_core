/**
 * Copyright (c);2025 Huawei Device Co., Ltd.
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
    const suite = new ArkTestsuite("Intl. NumberFormatOptions ResolvedNumberFormatOptions");
    suite.addTest("NumberFormatOptions check interface", testNumberFormatOptions);
    suite.addTest("ResolvedNumberFormatOptions check interface", testResolvedNumberFormatOptions);
    suite.addTest("OptionsNumberFormat read not default ", testOptionsReadNotDefault);
    suite.addTest("NumberFormatOptions test1 ", testNumberFormatOptions1);
    suite.addTest("NumberFormatOptions test2 ", testNumberFormatOptions2);
    suite.addTest("NumberFormatOptions test3 ", testNumberFormatOptions3);
    suite.addTest("NumberFormatOptions test4 ", testNumberFormatOptions4);
    suite.addTest("NumberFormatOptions test5 ", testNumberFormatOptions5);
    suite.addTest("NumberFormatOptions test6 ", testNumberFormatOptions6);
    suite.addTest("NumberFormatOptions test7 ", testNumberFormatOptions7);
    return suite.run();
}

function testNumberFormatOptions() {
    let x: Intl.NumberFormatOptions = {};
    assertEQ(x.compactDisplay, undefined);
    assertEQ(x.currency, undefined);
    assertEQ(x.currencyDisplay, undefined);
    assertEQ(x.currencySign, undefined);
    assertEQ(x.localeMatcher, undefined);
    assertEQ(x.maximumFractionDigits, undefined);
    assertEQ(x.maximumSignificantDigits, undefined);
    assertEQ(x.minimumFractionDigits, undefined);
    assertEQ(x.minimumIntegerDigits, undefined);
    assertEQ(x.minimumSignificantDigits, undefined);
    assertEQ(x.notation, undefined);
    assertEQ(x.signDisplay, undefined);
    assertEQ(x.style, undefined);
    assertEQ(x.unit, undefined);
    assertEQ(x.unitDisplay, undefined);
    assertEQ(x.useGrouping, undefined);
}

function testResolvedNumberFormatOptions() {
    let x: Intl.ResolvedNumberFormatOptions = {
        // TS requires to define the following properties
        locale: "",
        style: "decimal",
        numberingSystem: "latn",
        minimumIntegerDigits: 1,
        useGrouping: false,
        notation: "compact",
        signDisplay: "auto",
        // The following properties couldn't be "undefined", so define them by example:
        maximumFractionDigits: 0,
        minimumFractionDigits: 0
    };
    assertEQ(x.compactDisplay, undefined);
    assertEQ(x.currencySign, undefined);
    assertEQ(x.currency, undefined);
    assertEQ(x.currencyDisplay, undefined);
    assertEQ(x.locale, "");
    assertEQ(x.maximumFractionDigits, 0);
    assertEQ(x.maximumSignificantDigits, undefined);
    assertEQ(x.minimumFractionDigits, 0);
    assertEQ(x.minimumIntegerDigits, 1);
    assertEQ(x.minimumSignificantDigits, undefined);
    assertEQ(x.notation, "compact");
    assertEQ(x.numberingSystem, "latn");
    assertEQ(x.signDisplay, "auto");
    assertEQ(x.style, "decimal");
    assertEQ(x.unit, undefined);
    assertEQ(x.unitDisplay, undefined);
    assertEQ(x.useGrouping, false);
}

function testOptionsReadNotDefault() {
    let options: Intl.NumberFormatOptions = {
        style: "decimal",
        useGrouping: false,
        notation: "compact",
        compactDisplay: "long",
        signDisplay: "auto",
        maximumFractionDigits: 21,
        maximumSignificantDigits: 20,
        minimumFractionDigits: 19,
        minimumIntegerDigits: 18,
        minimumSignificantDigits: 17
    };

    let x = new Intl.NumberFormat("zh", options).resolvedOptions();
    assertEQ(x.compactDisplay, "long");
    assertEQ(x.currencySign, undefined);
    assertEQ(x.currency, undefined);
    assertEQ(x.currencyDisplay, undefined);
    assertEQ(x.locale, "zh");
    assertEQ(x.maximumFractionDigits, 21);
    assertEQ(x.maximumSignificantDigits, 20);
    assertEQ(x.minimumFractionDigits, 19);
    assertEQ(x.minimumIntegerDigits, 18);
    assertEQ(x.minimumSignificantDigits, 17);
    assertEQ(x.notation, "compact");
    assertEQ(x.numberingSystem, "latn");
    assertEQ(x.signDisplay, "auto");
    assertEQ(x.style, "decimal");
    assertEQ(x.unit, undefined);
    assertEQ(x.unitDisplay, undefined);
    assertEQ(x.useGrouping, false);
}

function testNumberFormatOptions1() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
        currency: "test"
    }
    let expectedError = new RangeError("Invalid currency");
    expectError((): void throws => { new Intl.NumberFormat(locales, options) }, expectedError);
}

function testNumberFormatOptions2() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
        currencySign: "standard"
    }
    let numberFormat = new Intl.NumberFormat(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    assertEQ(formattedNumber, "1,234,567.89");
}

function testNumberFormatOptions3() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
        currencySign: undefined
    }
    let numberFormat = new Intl.NumberFormat(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    assertEQ(formattedNumber, "1,234,567.89");
}

function testNumberFormatOptions4() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
        minimumIntegerDigits: 21
    }
    let numberFormat = new Intl.NumberFormat(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    assertEQ(formattedNumber, "000,000,000,000,001,234,567.89");
}

function testNumberFormatOptions5() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
        style: "currency",
        currency: "CNY",
        currencySign: "accounting"
    }
    let numberFormat = new Intl.NumberFormat(locales, options);
    let formattedNumber = numberFormat.format(-1234567.89);
    assertEQ(formattedNumber, "(¥1,234,567.89)");
}

function testNumberFormatOptions6() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
        useGrouping: false
    }
    let numberFormat = new Intl.NumberFormat(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    assertEQ(formattedNumber, "1234567.89");
}

function testNumberFormatOptions7() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
        minimumIntegerDigits: 10
    }
    let numberFormat = new Intl.NumberFormat(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    assertEQ(formattedNumber, "0,001,234,567.89");
}
