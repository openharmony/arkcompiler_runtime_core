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
    const suite = new ArkTestsuite("Intl. NumberFormatOptions ResolvedNumberFormatOptions check possibility to read fields");
    suite.addTest("testNumberFormatOptions", testNumberFormatOptions);
    suite.addTest("testResolvedNumberFormatOptions", testResolvedNumberFormatOptions);
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
