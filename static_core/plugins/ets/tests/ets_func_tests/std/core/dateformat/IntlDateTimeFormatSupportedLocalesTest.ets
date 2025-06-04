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

function testResolvedOptionsRetrieval(): void {
    const locale = "en-US"
    const opts: Intl.DateTimeFormatOptions = { year: "numeric", month: "2-digit", day: "2-digit", hour: "2-digit", minute: "2-digit", timeZone: "UTC" }
    const formatter = new Intl.DateTimeFormat(locale, opts)
    const resolvedOpts = formatter.resolvedOptions()

    assertEQ(resolvedOpts.locale, locale)
    assertEQ(resolvedOpts.calendar, "gregorian")
    assertEQ(resolvedOpts.numberingSystem, "latn")
    assertEQ(resolvedOpts.timeZone, opts.timeZone)
    assertEQ(resolvedOpts.hourCycle, "h12")
    assertEQ(resolvedOpts.hour12, true)
}

function testSupportedLocalesOfStringArray(): void {
    const requestedLocales = ["en-US-u-hc-h23", "zh-CN", "foo", "en-US"]
    const supportedLocales = Intl.DateTimeFormat.supportedLocalesOf(requestedLocales)

    assertEQ(supportedLocales.length, 3)
    assertEQ(supportedLocales[0], requestedLocales[0])
    assertEQ(supportedLocales[1], requestedLocales[1])
    assertEQ(supportedLocales[2], requestedLocales[3])
}

function testSupportedLocalesOfString(): void {
    const requestedLocale = "en-US-u-hc-h23"
    const supportedLocales = Intl.DateTimeFormat.supportedLocalesOf(requestedLocale)

    assertEQ(supportedLocales.length, 1)
    assertEQ(supportedLocales[0], requestedLocale)
}

function testSupportedLocalesOfLocaleArray(): void {
    const requestedLocales = [new Intl.Locale("en-US-u-hc-h23"), new Intl.Locale("foo"), new Intl.Locale("zh-CN")]
    const supportedLocales = Intl.DateTimeFormat.supportedLocalesOf(requestedLocales)

    assertEQ(supportedLocales.length, 2)
    assertEQ(supportedLocales[0], requestedLocales[0].toString())
    assertEQ(supportedLocales[1], requestedLocales[2].toString())
}

function testSupportedLocalesOfLocale(): void {
    const requestedLocale = new Intl.Locale("en-US-u-hc-h23")
    const supportedLocales = Intl.DateTimeFormat.supportedLocalesOf(requestedLocale)

    assertEQ(supportedLocales.length, 1)
    assertEQ(supportedLocales[0], requestedLocale.toString())
}

function main(): int {
    const suite = new ArkTestsuite("Intl.DateTimeFormat")
    suite.addTest("resolved options retrieval", testResolvedOptionsRetrieval)
    suite.addTest("supportedLocalesOf(string[])", testSupportedLocalesOfStringArray)
    suite.addTest("supportedLocalesOf(string)", testSupportedLocalesOfString)
    suite.addTest("supportedLocalesOf(Locale[])", testSupportedLocalesOfLocaleArray)
    suite.addTest("supportedLocalesOf(Locale)", testSupportedLocalesOfLocale)
    return suite.run()
}
