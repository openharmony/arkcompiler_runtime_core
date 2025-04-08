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
    const suite = new ArkTestsuite("Intl. NumberFormatOptions");
    suite.addTest("NumberFormatOptions test1 ", testNumberFormatOptions1);
    suite.addTest("NumberFormatOptions test2 ", testNumberFormatOptions2);
    suite.addTest("NumberFormatOptions test3 ", testNumberFormatOptions3);
    suite.addTest("NumberFormatOptions test4 ", testNumberFormatOptions4);
    suite.addTest("NumberFormatOptions test5 ", testNumberFormatOptions5);
    suite.addTest("NumberFormatOptions test6 ", testNumberFormatOptions6);
    return suite.run();
}

function testNumberFormatOptions1() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
        useGrouping: true
    }
    let numberFormat = new Intl.NumberFormat(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    assertEQ(formattedNumber, "1,234,567.89");
}

function testNumberFormatOptions2() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
        useGrouping: undefined
    }
    let numberFormat = new Intl.NumberFormat(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    assertEQ(formattedNumber, "1,234,567.89");
}

function testNumberFormatOptions3() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
        minimumIntegerDigits: 1
    }
    let numberFormat = new Intl.NumberFormat(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    assertEQ(formattedNumber, "1,234,567.89");
}

function testNumberFormatOptions4() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
        minimumIntegerDigits: undefined
    }
    let numberFormat = new Intl.NumberFormat(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    assertEQ(formattedNumber, "1,234,567.89");
}

function testNumberFormatOptions5() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
        minimumFractionDigits: undefined
    }
    let numberFormat = new Intl.NumberFormat(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    assertEQ(formattedNumber, "1,234,567.89");
}

function testNumberFormatOptions6() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
        maximumSignificantDigits: undefined
    }
    let numberFormat = new Intl.NumberFormat(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    assertEQ(formattedNumber, "1,234,567.89");
}
