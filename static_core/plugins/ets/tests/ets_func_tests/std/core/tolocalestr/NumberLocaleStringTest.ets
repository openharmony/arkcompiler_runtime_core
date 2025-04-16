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
}

function testNaNToLocaleString(): void {
    const nan: Number = Number.NaN
    const negNan: Number = -Number.NaN

    assertEQ(nan.toLocaleString("en-US"), "NaN")
    assertEQ(negNan.toLocaleString("en-US"), "NaN")

    assertEQ(nan.toLocaleString("ru-RU"), "не число")
    assertEQ(nan.toLocaleString("fa-IR"), "ناعدد")
}

function main(): int {
    const suite = new ArkTestsuite("Number")
    suite.addTest("toLocaleString", testNumberToLocaleString)
    suite.addTest("NaN.toLocaleString", testNaNToLocaleString)
    return suite.run()
}
