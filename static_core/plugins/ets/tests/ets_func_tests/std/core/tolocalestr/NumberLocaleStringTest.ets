/*
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

function testNumberToLocaleString(): void {
    const langTag = "zh-CN"
    const options: Intl.NumberFormatOptions = { useGrouping: false }

    const locale = new Intl.Locale(langTag)
    const num = new Number(45678)

    assertEQ(num.toLocaleString(langTag, options), "45678")
    assertEQ(num.toLocaleString(locale, options), "45678")

    assertEQ(num.toLocaleString(langTag), "45,678")
    assertEQ(num.toLocaleString(locale), "45,678")
    assertEQ(num.toLocaleString(), num.toLocaleString(undefined))
}

function testNaNToLocaleString(): void {
    const nan: Number = Number.NaN
    const negNan: Number = -Number.NaN

    assertEQ(nan.toLocaleString("en-US"), "NaN")
    assertEQ(negNan.toLocaleString("en-US"), "NaN")

    assertEQ(nan.toLocaleString("ru-RU"), "не число")
    assertEQ(nan.toLocaleString("fa-IR"), "ناعدد")
}

function testCurrencyToLocaleString(): void {
    const num: Number = 123456.789;

    const usdResult = num.toLocaleString("en-US", { style: "currency", currency: "USD" });
    assertEQ(usdResult, "$123,456.79", "USD format");

    const jpyResult = num.toLocaleString("ja-JP", { style: "currency", currency: "JPY" });
    assertEQ(jpyResult, "￥123,457", "JPY format");

    const cnyResult = num.toLocaleString("zh-CN", { style: "currency", currency: "CNY" });
    assertEQ(cnyResult, "¥123,456.79", "CNY format");

    const gbpResult = num.toLocaleString("en-GB", { style: "currency", currency: "GBP" });
    assertEQ(gbpResult, "£123,456.79", "GBP format");

    const inrResult = num.toLocaleString("en-IN", { style: "currency", currency: "INR" });
    assertEQ(inrResult, "₹1,23,456.79", "INR format");
}

function main(): int {
    const suite = new ArkTestsuite("Number")
    suite.addTest("toLocaleString", testNumberToLocaleString)
    suite.addTest("NaN.toLocaleString", testNaNToLocaleString)
    suite.addTest("num.toLocaleString", testCurrencyToLocaleString)
    return suite.run()
}
