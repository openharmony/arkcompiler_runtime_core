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
    const suite = new ArkTestsuite("Intl. NumberFormat format with NumberFormatOptions");
    // Tests from https://gitee.com/openharmony/xts_acts/tree/master/commonlibrary/ets_utils/codeparsingtest/builtinstest/entry/src/ohosTest/ets/test/Intl/Intl.test.ets
    suite.addTest("testNumberFormat0001", testNumberFormat0001);
    suite.addTest("testNumberFormat0002", testNumberFormat0002);
    suite.addTest("testNumberFormat0003", testNumberFormat0003);
    suite.addTest("testNumberFormat0004", testNumberFormat0004);
    suite.addTest("testNumberFormat0005", testNumberFormat0005);
    suite.addTest("testNumberFormat0006", testNumberFormat0006);
    return suite.run();
}

function testNumberFormat0001() {
    let format = Intl.NumberFormat.$_invoke().format(100);
    assertEQ(format, "100");
}

function testNumberFormat0002() {
    let format = Intl.NumberFormat.$_invoke("de").format(100);
    assertEQ(format, "100");
}

function testNumberFormat0003() {
    let options: Intl.NumberFormatOptions = {
        style: "currency", currency: 'RUB'
    }
    let format = Intl.NumberFormat.$_invoke('ru-RU', options).format(100);
    assertEQ(format, "100,00\xa0₽");
}

function testNumberFormat0004() {
    let options: Intl.NumberFormatOptions = {
        style: "currency", currency: "CNY"
    }
    let format = Intl.NumberFormat.$_invoke("zh", options).format(100);
    assertEQ(format, "¥100.00");
}

function testNumberFormat0005() {
    let options: Intl.NumberFormatOptions = {
        style: "currency", currency: "CNY", minimumIntegerDigits: 4
    }
    let format = Intl.NumberFormat.$_invoke("zh", options).format(100);
    assertEQ(format, "¥0,100.00");
}

function testNumberFormat0006() {
    let options: Intl.NumberFormatOptions = {
        style: "percent"
    }
    let format = Intl.NumberFormat.$_invoke("de", options).format(100);
    assertEQ(format, "10.000\xa0%");
}
