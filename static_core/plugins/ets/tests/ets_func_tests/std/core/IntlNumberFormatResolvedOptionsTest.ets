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
    let suite = new ArkTestsuite("Intl. NumberFormat resolvedOptions");
    suite.addTest("testResolvedOptionsDefault", testResolvedOptionsDefault);
    suite.addTest("testResolvedOptionsNotDefault", testResolvedOptionsNotDefault);
    suite.addTest("testResolvedOptionsDecimal", testResolvedOptionsDecimal);
    suite.addTest("testResolvedOptionsPercent", testResolvedOptionsPercent);
    suite.addTest("testResolvedOptionsCurrency", testResolvedOptionsCurrency);
    //Tests are from https://gitee.com/openharmony/xts_acts/tree/master/commonlibrary/ets_utils/codeparsingtest/builtinstest/entry/src/ohosTest/ets/test/Intl/Intl.test.ets
    suite.addTest("testResolvedOptions0001", testResolvedOptions0001);
    suite.addTest("testResolvedOptions0002", testResolvedOptions0002);
    suite.addTest("testResolvedOptions0003", testResolvedOptions0003);
    suite.addTest("testResolvedOptions0004", testResolvedOptions0004);
    suite.addTest("testResolvedOptions0005", testResolvedOptions0005);
    suite.addTest("testResolvedOptions0006", testResolvedOptions0006);
    suite.addTest("testResolvedOptions0007", testResolvedOptions0007);
    return suite.run();
}

function testResolvedOptions0001() {
    let options = Intl.NumberFormat.$_invoke().resolvedOptions();
    assertTrue(options.locale.includes('en-US'));
    assertEQ(options.numberingSystem, 'latn');
    assertEQ(options.style, 'decimal');
    assertEQ(options.minimumIntegerDigits, 1);
    assertEQ(options.minimumFractionDigits, 0);
    assertEQ(options.maximumFractionDigits, 3);
    assertEQ(options.useGrouping, true);
    assertEQ(options.notation, "standard");
    assertEQ(options.signDisplay, "auto");
}

function testResolvedOptions0002() {
    let options = Intl.NumberFormat.$_invoke("de").resolvedOptions();
    assertEQ(options.locale, "de");
}

function testResolvedOptions0003() {
    let options: Intl.NumberFormatOptions = {
        style: "currency", currency: "EUR"
    }
    let getOptions = new Intl.NumberFormat("de", options).resolvedOptions();
    assertEQ(getOptions.locale, 'de');
    assertEQ(getOptions.numberingSystem, 'latn');
    assertEQ(getOptions.style, 'currency');
    assertEQ(getOptions.currency, 'EUR');
    assertEQ(getOptions.currencyDisplay, 'symbol');
    assertEQ(getOptions.currencySign, 'standard');
    assertEQ(getOptions.minimumIntegerDigits, 1);
    assertEQ(getOptions.minimumFractionDigits, 2);
    assertEQ(getOptions.maximumFractionDigits, 2);
    assertEQ(getOptions.useGrouping, true);
    assertEQ(getOptions.notation, 'standard');
    assertEQ(getOptions.signDisplay, 'auto');
}

function testResolvedOptions0004() {
    let options: Intl.NumberFormatOptions = {
        style: "percent"
    }
    let getOptions = new Intl.NumberFormat("de", options).resolvedOptions();
    assertEQ(getOptions.locale, 'de');
    assertEQ(getOptions.numberingSystem, 'latn');
    assertEQ(getOptions.style, 'percent');
    assertEQ(getOptions.minimumIntegerDigits, 1);
    assertEQ(getOptions.minimumFractionDigits, 0);
    assertEQ(getOptions.maximumFractionDigits, 0);
    assertEQ(getOptions.useGrouping, true);
    assertEQ(getOptions.notation, 'standard');
    assertEQ(getOptions.signDisplay, 'auto');
}

function testResolvedOptions0005() {
    let options: Intl.NumberFormatOptions = {
        style: "currency", currency: "EUR", currencySign: "accounting"
    }
    let getOptions = new Intl.NumberFormat("de", options).resolvedOptions();
    assertEQ(getOptions.locale, 'de');
    assertEQ(getOptions.numberingSystem, 'latn');
    assertEQ(getOptions.style, 'currency');
    assertEQ(getOptions.currency, 'EUR');
    assertEQ(getOptions.currencyDisplay, 'symbol');
    assertEQ(getOptions.currencySign, 'accounting');
    assertEQ(getOptions.minimumIntegerDigits, 1);
    assertEQ(getOptions.minimumFractionDigits, 2);
    assertEQ(getOptions.maximumFractionDigits, 2);
    assertEQ(getOptions.useGrouping, true);
    assertEQ(getOptions.notation, 'standard');
    assertEQ(getOptions.signDisplay, 'auto');
}

function testResolvedOptions0006() {
    let options: Intl.NumberFormatOptions = {
        useGrouping: false
    }
    let getOptions = new Intl.NumberFormat("de", options).resolvedOptions();
    assertEQ(getOptions.locale, 'de');
    assertEQ(getOptions.numberingSystem, 'latn');
    assertEQ(getOptions.style, 'decimal');
    assertEQ(getOptions.minimumIntegerDigits, 1);
    assertEQ(getOptions.minimumFractionDigits, 0);
    assertEQ(getOptions.maximumFractionDigits, 3);
    assertEQ(getOptions.useGrouping, false);
    assertEQ(getOptions.notation, 'standard');
    assertEQ(getOptions.signDisplay, 'auto');
}

function testResolvedOptions0007() {
    let options: Intl.NumberFormatOptions = {
        minimumIntegerDigits: 10
    }
    let getOptions = new Intl.NumberFormat("de", options).resolvedOptions();
    assertEQ(getOptions.minimumIntegerDigits, 10);
}

function testResolvedOptionsDefault() {
    let expected: Intl.ResolvedNumberFormatOptions = {
            locale: "en",
            numberingSystem: "latn",
            style: "decimal",
            useGrouping: true,
            notation: "standard",
            signDisplay: "auto",
            maximumFractionDigits: 3,
            minimumFractionDigits: 0,
            minimumIntegerDigits: 1
        }
    let result = new Intl.NumberFormat("en", undefined).resolvedOptions();
    let isEqualed = equaledResolvedNumberFormatOptions(result, expected);
    assertTrue(isEqualed);
    result = new Intl.NumberFormat("en").resolvedOptions();
    isEqualed = equaledResolvedNumberFormatOptions(result, expected);
    assertTrue(isEqualed);
}

function testResolvedOptionsNotDefault() {
    let options: Intl.NumberFormatOptions = {
        style: "decimal",
        useGrouping: false,
        notation: "compact",
        compactDisplay: "long",
        signDisplay: "auto",
        minimumFractionDigits: 19,
        maximumFractionDigits: 21,
        minimumSignificantDigits: 17,
        maximumSignificantDigits: 20,
        minimumIntegerDigits: 18
    };

    let x = new Intl.NumberFormat("zh", options).resolvedOptions();
    assertEQ(x.compactDisplay, "long");
    assertEQ(x.currencySign, undefined);
    assertEQ(x.currency, undefined);
    assertEQ(x.currencyDisplay, undefined);
    assertEQ(x.locale, "zh");
    assertEQ(x.minimumFractionDigits, 19);
    assertEQ(x.maximumFractionDigits, 21);
    assertEQ(x.minimumSignificantDigits, undefined);
    assertEQ(x.maximumSignificantDigits, undefined);
    assertEQ(x.minimumIntegerDigits, 18);
    assertEQ(x.notation, "compact");
    assertEQ(x.numberingSystem, "latn");
    assertEQ(x.signDisplay, "auto");
    assertEQ(x.style, "decimal");
    assertEQ(x.unit, undefined);
    assertEQ(x.unitDisplay, undefined);
    assertEQ(x.useGrouping, false);
}


function testResolvedOptionsDecimal() {
    let options: Intl.NumberFormatOptions = {
        style: "decimal"
    }
    let getOptions = new Intl.NumberFormat("de", options).resolvedOptions();
    assertEQ(getOptions.locale, "de");
    assertEQ(getOptions.numberingSystem, "latn");
    assertEQ(getOptions.style, "decimal");
    assertEQ(getOptions.minimumIntegerDigits, 1.0);
    assertEQ(getOptions.minimumFractionDigits, 0.0);
    assertEQ(getOptions.maximumFractionDigits, 3.0);
    assertEQ(getOptions.useGrouping, true);
    assertEQ(getOptions.notation, "standard");
    assertEQ(getOptions.signDisplay, "auto");
}

function testResolvedOptionsPercent() {
    let options: Intl.NumberFormatOptions = {
        style: "percent"
    }
    let getOptions = new Intl.NumberFormat("de", options).resolvedOptions();
    assertEQ(getOptions.locale, "de");
    assertEQ(getOptions.numberingSystem, "latn");
    assertEQ(getOptions.style, "percent");
    assertEQ(getOptions.minimumIntegerDigits, 1.0);
    assertEQ(getOptions.minimumFractionDigits, 0.0);
    assertEQ(getOptions.maximumFractionDigits, 0.0);
    assertEQ(getOptions.useGrouping, true)
    assertEQ(getOptions.notation, "standard");
    assertEQ(getOptions.signDisplay, "auto");
}

function testResolvedOptionsCurrency() {
    let options: Intl.NumberFormatOptions = {
        style: "currency", currency: "CNY"
    }
    let getOptions = new Intl.NumberFormat("zh", options).resolvedOptions();
    assertEQ(getOptions.locale, "zh");
    assertEQ(getOptions.numberingSystem, "latn");
    assertEQ(getOptions.style, "currency");
    assertEQ(getOptions.currency, "CNY");
    assertEQ(getOptions.currencyDisplay, "symbol");
    assertEQ(getOptions.currencySign, "standard");
    assertEQ(getOptions.minimumIntegerDigits, 1.0);
    assertEQ(getOptions.minimumFractionDigits, 2.0);
    assertEQ(getOptions.maximumFractionDigits, 2.0);
    assertEQ(getOptions.useGrouping, true);
    assertEQ(getOptions.notation, "standard");
    assertEQ(getOptions.signDisplay, "auto");
}

function equaledResolvedNumberFormatOptions(a: Intl.ResolvedNumberFormatOptions, b: Intl.ResolvedNumberFormatOptions): boolean {
    assertEQ(a.compactDisplay,b.compactDisplay);
    assertEQ(a.currencySign, b.currencySign);
    assertEQ(a.currency, b.currency);
    assertEQ(a.currencyDisplay, b.currencyDisplay);
    assertEQ(a.locale, b.locale);
    assertEQ(a.maximumFractionDigits, b.maximumFractionDigits);
    assertEQ(a.maximumSignificantDigits, b.maximumSignificantDigits);
    assertEQ(a.minimumFractionDigits, b.minimumFractionDigits);
    assertEQ(a.minimumIntegerDigits, b.minimumIntegerDigits);
    assertEQ(a.minimumSignificantDigits, b.minimumSignificantDigits);
    assertEQ(a.notation, b.notation);
    assertEQ(a.numberingSystem, b.numberingSystem);
    assertEQ(a.signDisplay, b.signDisplay);
    assertEQ(a.style, b.style);
    assertEQ(a.unit, b.unit);
    assertEQ(a.unitDisplay, b.unitDisplay);
    assertEQ(a.useGrouping, b.useGrouping);
    return true;
}
