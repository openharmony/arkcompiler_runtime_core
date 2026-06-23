/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

/*
 * Tests for DateTimeFormat.resolvedOptions() with proper field assertions.
 *
 * Verifies:
 * - Calendar name mapping (gregorian→gregory, ethiopic-amete-alem→ethioaa)
 * - Hour cycle derivation from locale keyword (-u-hc-hXX)
 * - hour12 boolean derivation from hourCycle
 * - zh-CN/zh-TW forced h23 workaround (bug H8)
 * - Numbering system resolution
 * - Option propagation (year, month, day, hour, minute, second, etc.)
 * - TimeZone resolution
 */

// --- Calendar name mapping tests ---

// gregorian → "gregory"
function testCalendarGregoryMapping(): void {
    let dtf = new Intl.DateTimeFormat('en-US', { timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.calendar, 'gregory', 'gregorian should map to \'gregory\'')
}

// ethiopic-amete-alem → "ethioaa"
function testCalendarEthioaaMapping(): void {
    let dtf = new Intl.DateTimeFormat('am-ET-u-ca-ethioaa', { timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.calendar, 'ethioaa', 'ethiopic-amete-alem should map to \'ethioaa\'')
}

// --- Hour cycle tests ---

// Explicit h12 via BCP47 extension
function testHourCycleH12Explicit(): void {
    let dtf = new Intl.DateTimeFormat('en-US-u-hc-h12', { hour: 'numeric', timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.hourCycle, 'h12', 'explicit -u-hc-h12 should resolve to h12')
    arktest.assertTrue(opts.hour12!, 'h12 should set hour12=true')
}

// Explicit h23 via BCP47 extension
function testHourCycleH23Explicit(): void {
    let dtf = new Intl.DateTimeFormat('en-US-u-hc-h23', { hour: 'numeric', timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.hourCycle, 'h23', 'explicit -u-hc-h23 should resolve to h23')
    arktest.assertFalse(opts.hour12!, 'h23 should set hour12=false')
}

// Explicit h11 via BCP47 extension
function testHourCycleH11Explicit(): void {
    let dtf = new Intl.DateTimeFormat('en-US-u-hc-h11', { hour: 'numeric', timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.hourCycle, 'h11', 'explicit -u-hc-h11 should resolve to h11')
    arktest.assertTrue(opts.hour12!, 'h11 should set hour12=true')
}

// Explicit h24 via BCP47 extension
function testHourCycleH24Explicit(): void {
    let dtf = new Intl.DateTimeFormat('en-US-u-hc-h24', { hour: 'numeric', timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.hourCycle, 'h24', 'explicit -u-hc-h24 should resolve to h24')
    arktest.assertFalse(opts.hour12!, 'h24 should set hour12=false')
}

// --- zh* hour cycle workaround tests (bug H8) ---

// --- zh locale hour cycle tests ---
// Mainland China variants should get h23 via the workaround.

function testZhGenericH23(): void {
    let dtf = new Intl.DateTimeFormat('zh', { hour: 'numeric', timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.hourCycle, 'h23', 'zh (no region) should get h23')
    arktest.assertFalse(opts.hour12!, 'zh should have hour12=false')
}

function testZhCNH23(): void {
    let dtf = new Intl.DateTimeFormat('zh-CN', { hour: 'numeric', timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.hourCycle, 'h23', 'zh-CN should get h23')
    arktest.assertFalse(opts.hour12!, 'zh-CN should have hour12=false')
}

function testZhHansH23(): void {
    let dtf = new Intl.DateTimeFormat('zh-Hans', { hour: 'numeric', timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.hourCycle, 'h23', 'zh-Hans (no region) should get h23')
    arktest.assertFalse(opts.hour12!, 'zh-Hans should have hour12=false')
}

function testZhHansCNH23(): void {
    let dtf = new Intl.DateTimeFormat('zh-Hans-CN', { hour: 'numeric', timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.hourCycle, 'h23', 'zh-Hans-CN should get h23')
    arktest.assertFalse(opts.hour12!, 'zh-Hans-CN should have hour12=false')
}

// Non-mainland zh variants should keep locale-default hour cycle (h12).

function testZhTWH12(): void {
    let dtf = new Intl.DateTimeFormat('zh-TW', { hour: 'numeric', timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.hourCycle, 'h12', 'zh-TW should use h12 (not forced h23)')
    arktest.assertTrue(opts.hour12!, 'zh-TW should have hour12=true')
}

function testZhHKH12(): void {
    let dtf = new Intl.DateTimeFormat('zh-HK', { hour: 'numeric', timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.hourCycle, 'h12', 'zh-HK should use h12')
    arktest.assertTrue(opts.hour12!, 'zh-HK should have hour12=true')
}

function testZhMOH12(): void {
    let dtf = new Intl.DateTimeFormat('zh-MO', { hour: 'numeric', timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.hourCycle, 'h12', 'zh-MO should use h12')
    arktest.assertTrue(opts.hour12!, 'zh-MO should have hour12=true')
}

function testZhSGH12(): void {
    let dtf = new Intl.DateTimeFormat('zh-SG', { hour: 'numeric', timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.hourCycle, 'h12', 'zh-SG should use h12')
    arktest.assertTrue(opts.hour12!, 'zh-SG should have hour12=true')
}

function testZhHantTWH12(): void {
    let dtf = new Intl.DateTimeFormat('zh-Hant-TW', { hour: 'numeric', timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.hourCycle, 'h12', 'zh-Hant-TW should use h12')
    arktest.assertTrue(opts.hour12!, 'zh-Hant-TW should have hour12=true')
}

// zh-TW with explicit hour12:true should override the workaround
function testZhTWExplicitHour12Override(): void {
    let dtf = new Intl.DateTimeFormat('zh-TW', { hour: 'numeric', timeZone: 'UTC', hour12: true })
    let opts = dtf.resolvedOptions()

    // Explicit hour12 option should override the workaround
    arktest.assertTrue(opts.hour12!, 'explicit hour12:true should override workaround')
    // When hour12 is true, hourCycle should be h11 or h12
    arktest.assertTrue(
        opts.hourCycle == 'h11' || opts.hourCycle == 'h12',
        `explicit hour12:true should resolve to h11/h12, got ${opts.hourCycle}`
    )
}

// zh-HK with explicit hourCycle should override the workaround
function testZhHKExplicitHourCycleOverride(): void {
    let dtf = new Intl.DateTimeFormat('zh-HK-u-hc-h12', { hour: 'numeric', timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()

    // Explicit -u-hc-h12 should be respected even for zh* locales
    arktest.assertEQ(opts.hourCycle, 'h12', 'explicit -u-hc-h12 should override zh workaround')
    arktest.assertTrue(opts.hour12!, 'h12 should set hour12=true')
}

// --- Numbering system tests ---

function testNumberingSystemLatn(): void {
    let dtf = new Intl.DateTimeFormat('en-US', { timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.numberingSystem, 'latn', 'en-US should use latn numbering')
}

function testNumberingSystemArab(): void {
    let dtf = new Intl.DateTimeFormat('ar-EG', { timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.numberingSystem, 'arab', 'ar-EG should use arab numbering')
}

// --- Required fields tests ---

function testRequiredFieldsPresent(): void {
    let dtf = new Intl.DateTimeFormat('en-US', { timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()

    // These 4 fields are required per the interface
    arktest.assertTrue(opts.locale !== undefined, 'locale must be defined')
    arktest.assertTrue(opts.calendar !== undefined, 'calendar must be defined')
    arktest.assertTrue(opts.numberingSystem !== undefined, 'numberingSystem must be defined')
    arktest.assertTrue(opts.timeZone !== undefined, 'timeZone must be defined')

    // Verify types
    arktest.assertEQ(opts.locale, 'en-US', 'locale should match input')
}

function testTimeZone(): void {
    let dtf = new Intl.DateTimeFormat('en-US', { timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.timeZone, 'UTC', 'timeZone should be UTC')
}

// --- Option propagation tests ---

function testOptionPropagationYearMonthDay(): void {
    let dtf = new Intl.DateTimeFormat('en-US', {
        year: 'numeric',
        month: 'long',
        day: '2-digit',
        timeZone: 'UTC'
    })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.year, 'numeric', 'year option should propagate')
    arktest.assertEQ(opts.month, 'long', 'month option should propagate')
    arktest.assertEQ(opts.day, '2-digit', 'day option should propagate')
}

function testOptionPropagationHourMinuteSecond(): void {
    let dtf = new Intl.DateTimeFormat('en-US', {
        hour: '2-digit',
        minute: 'numeric',
        second: '2-digit',
        timeZone: 'UTC'
    })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.hour, '2-digit', 'hour option should propagate')
    arktest.assertEQ(opts.minute, 'numeric', 'minute option should propagate')
    arktest.assertEQ(opts.second, '2-digit', 'second option should propagate')
}

function testOptionPropagationDateStyleTimeStyle(): void {
    let dtf = new Intl.DateTimeFormat('en-US', {
        dateStyle: 'full',
        timeStyle: 'long',
        timeZone: 'UTC'
    })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.dateStyle, 'full', 'dateStyle should propagate')
    arktest.assertEQ(opts.timeStyle, 'long', 'timeStyle should propagate')
}

function testOptionPropagationWeekdayEra(): void {
    let dtf = new Intl.DateTimeFormat('en-US', {
        weekday: 'long',
        era: 'short',
        timeZone: 'UTC'
    })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.weekday, 'long', 'weekday should propagate')
    arktest.assertEQ(opts.era, 'short', 'era should propagate')
}

function testOptionPropagationTimeZoneName(): void {
    let dtf = new Intl.DateTimeFormat('en-US', {
        timeZoneName: 'short',
        timeZone: 'UTC'
    })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.timeZoneName, 'short', 'timeZoneName should propagate')
}

function testOptionPropagationFractionalSecondDigits(): void {
    let dtf = new Intl.DateTimeFormat('en-US', {
        fractionalSecondDigits: 3,
        second: '2-digit',
        timeZone: 'UTC'
    })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.fractionalSecondDigits, 3, 'fractionalSecondDigits should propagate')
}

// --- hour12 derivation correctness ---

function testHour12DerivationFromH12(): void {
    let dtf = new Intl.DateTimeFormat('en-US-u-hc-h12', { hour: 'numeric', timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.hourCycle, 'h12', '')
    arktest.assertTrue(opts.hour12!, 'h12 → hour12=true')
}

function testHour12DerivationFromH23(): void {
    let dtf = new Intl.DateTimeFormat('en-US-u-hc-h23', { hour: 'numeric', timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.hourCycle, 'h23', '')
    arktest.assertFalse(opts.hour12!, 'h23 → hour12=false')
}

function testHour12DerivationFromH11(): void {
    let dtf = new Intl.DateTimeFormat('en-US-u-hc-h11', { hour: 'numeric', timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.hourCycle, 'h11', '')
    arktest.assertTrue(opts.hour12!, 'h11 → hour12=true')
}

function testHour12DerivationFromH24(): void {
    let dtf = new Intl.DateTimeFormat('en-US-u-hc-h24', { hour: 'numeric', timeZone: 'UTC' })
    let opts = dtf.resolvedOptions()
    arktest.assertEQ(opts.hourCycle, 'h24', '')
    arktest.assertFalse(opts.hour12!, 'h24 → hour12=false')
}

// --- NumberFormat.resolvedOptions ---

function testNfResolvedOptionsDefaults(): void {
    let nf = new Intl.NumberFormat('en-US')
    let opts = nf.resolvedOptions()
    arktest.assertEQ(opts.locale, 'en-US', 'NF locale should match')
    arktest.assertEQ(opts.numberingSystem, 'latn', 'NF en-US numberingSystem')
    arktest.assertEQ(opts.style, 'decimal', 'NF default style')
    arktest.assertEQ(opts.minimumIntegerDigits, 1, 'NF default minInt')
    arktest.assertEQ(opts.minimumFractionDigits, 0, 'NF default minFrac')
    arktest.assertEQ(opts.maximumFractionDigits, 3, 'NF default maxFrac')
    arktest.assertTrue(opts.useGrouping, 'NF default useGrouping')
}

function testNfResolvedOptionsCurrency(): void {
    let nf = new Intl.NumberFormat('de-DE', { style: 'currency', currency: 'EUR' })
    let opts = nf.resolvedOptions()
    arktest.assertEQ(opts.style, 'currency', 'NF currency style')
    arktest.assertEQ(opts.currency, 'EUR', 'NF currency')
    arktest.assertEQ(opts.currencyDisplay, 'symbol', 'NF default currencyDisplay')
    arktest.assertEQ(opts.currencySign, 'standard', 'NF default currencySign')
    arktest.assertEQ(opts.minimumFractionDigits, 2, 'NF currency default minFrac')
    arktest.assertEQ(opts.maximumFractionDigits, 2, 'NF currency default maxFrac')
}

function testNfResolvedOptionsPercent(): void {
    let nf = new Intl.NumberFormat('en', { style: 'percent' })
    let opts = nf.resolvedOptions()
    arktest.assertEQ(opts.style, 'percent', 'NF percent style')
    arktest.assertEQ(opts.minimumFractionDigits, 0, 'NF percent default minFrac')
    arktest.assertEQ(opts.maximumFractionDigits, 0, 'NF percent default maxFrac')
}

function main(): int {
    const suite = new arktest.ArkTestsuite('resolvedOptions tests')

    // Calendar mapping
    suite.addTest('calendar gregory mapping', testCalendarGregoryMapping)
    suite.addTest('calendar ethioaa mapping', testCalendarEthioaaMapping)

    // Hour cycle — explicit BCP47
    suite.addTest('hourCycle h12 explicit', testHourCycleH12Explicit)
    suite.addTest('hourCycle h23 explicit', testHourCycleH23Explicit)
    suite.addTest('hourCycle h11 explicit', testHourCycleH11Explicit)
    suite.addTest('hourCycle h24 explicit', testHourCycleH24Explicit)

    // zh locale hour cycle — mainland gets h23, others keep locale default h12
    suite.addTest('zh (generic) h23', testZhGenericH23)
    suite.addTest('zh-CN h23', testZhCNH23)
    suite.addTest('zh-Hans h23', testZhHansH23)
    suite.addTest('zh-Hans-CN h23', testZhHansCNH23)
    suite.addTest('zh-TW h12', testZhTWH12)
    suite.addTest('zh-HK h12', testZhHKH12)
    suite.addTest('zh-MO h12', testZhMOH12)
    suite.addTest('zh-SG h12', testZhSGH12)
    suite.addTest('zh-Hant-TW h12', testZhHantTWH12)
    suite.addTest('zh-TW explicit hour12 override', testZhTWExplicitHour12Override)
    suite.addTest('zh-HK explicit hourCycle override', testZhHKExplicitHourCycleOverride)

    // Numbering system
    suite.addTest('numberingSystem latn', testNumberingSystemLatn)
    suite.addTest('numberingSystem arab', testNumberingSystemArab)

    // Required fields
    suite.addTest('required fields present', testRequiredFieldsPresent)
    suite.addTest('timeZone', testTimeZone)

    // Option propagation
    suite.addTest('option propagation year/month/day', testOptionPropagationYearMonthDay)
    suite.addTest('option propagation hour/minute/second', testOptionPropagationHourMinuteSecond)
    suite.addTest('option propagation dateStyle/timeStyle', testOptionPropagationDateStyleTimeStyle)
    suite.addTest('option propagation weekday/era', testOptionPropagationWeekdayEra)
    suite.addTest('option propagation timeZoneName', testOptionPropagationTimeZoneName)
    suite.addTest('option propagation fractionalSecondDigits', testOptionPropagationFractionalSecondDigits)

    // hour12 derivation
    suite.addTest('hour12 derivation h12', testHour12DerivationFromH12)
    suite.addTest('hour12 derivation h23', testHour12DerivationFromH23)
    suite.addTest('hour12 derivation h11', testHour12DerivationFromH11)
    suite.addTest('hour12 derivation h24', testHour12DerivationFromH24)

    // NumberFormat resolvedOptions
    suite.addTest('NF resolvedOptions defaults', testNfResolvedOptionsDefaults)
    suite.addTest('NF resolvedOptions currency', testNfResolvedOptionsCurrency)
    suite.addTest('NF resolvedOptions percent', testNfResolvedOptionsPercent)

    return suite.run()
}
