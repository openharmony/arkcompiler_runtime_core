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
    const suite = new ArkTestsuite("Intl. Interfaces NumberFormatOptions and ResolvedNumberFormatOptions, class NumberFormatPart");
    suite.addTest("NumberFormatOptions check interface", testNumberFormatOptions);
    suite.addTest("ResolvedNumberFormatOptions check interface", testResolvedNumberFormatOptions);
    return suite.run();
}

function testNumberFormatOptions() {
    let x: Intl.NumberFormatOptions = {};
    assertTrue(x.compactDisplay == undefined);
    assertTrue(x.currency == undefined);
    assertTrue(x.currencyDisplay == undefined);
    assertTrue(x.currencySign == undefined);
    assertTrue(x.localeMatcher == undefined);
    assertTrue(x.maximumFractionDigits == undefined);
    assertTrue(x.maximumSignificantDigits == undefined);
    assertTrue(x.minimumFractionDigits == undefined);
    assertTrue(x.minimumIntegerDigits == undefined);
    assertTrue(x.minimumSignificantDigits == undefined);
    assertTrue(x.notation == undefined);
    assertTrue(x.signDisplay == undefined);
    assertTrue(x.style == undefined);
    assertTrue(x.unit == undefined);
    assertTrue(x.unitDisplay == undefined);
    assertTrue(x.useGrouping == undefined);
}

function testResolvedNumberFormatOptions() {
    let x: Intl.ResolvedNumberFormatOptions = {
        // TS requires to define the following properties
        locale: "",
        style: "decimal",
        numberingSystem: "",
        minimumIntegerDigits: 1,
        useGrouping: false,
        notation: "compact",
        signDisplay: "auto",
        // The following properties couldn't be "undefined", so define them by example:
        maximumFractionDigits: 0,
        minimumFractionDigits: 0
    };
    assertTrue(x.compactDisplay == undefined);
    assertTrue(x.currencySign == undefined);
    assertTrue(x.currency == undefined);
    assertTrue(x.currencyDisplay == undefined);
    assertTrue(x.locale == "");
    assertTrue(x.maximumFractionDigits == 0);
    assertTrue(x.maximumSignificantDigits == undefined);
    assertTrue(x.minimumFractionDigits == 0);
    assertTrue(x.minimumIntegerDigits == 1);
    assertTrue(x.minimumSignificantDigits == undefined);
    assertTrue(x.notation == "compact");
    assertTrue(x.numberingSystem == "");
    assertTrue(x.signDisplay == "auto");
    assertTrue(x.style == "decimal");
    assertTrue(x.unit == undefined);
    assertTrue(x.unitDisplay == undefined);
    assertTrue(x.useGrouping == false);
}
