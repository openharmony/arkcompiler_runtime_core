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
 * Tests for formatRangeToParts with field-by-field assertions on type/value/source.
 * Covers DateTimeFormat.formatRangeToParts and NumberFormat.formatRangeToParts.
 *
 * Verifies:
 * - Correct source attribution (startRange/endRange/shared) for each part
 * - Literal separators within ranges vs between ranges
 * - Equal-value collapse behavior (NumberFormat)
 * - Negative range handling
 */

// --- Helpers ---

function assertDtfRangePart(part: Intl.DateTimeRangeFormatPart, eType: string, eValue: string,
                            eSource: string): void {
    arktest.assertEQ(part.type, eType, `DTF range type mismatch: expected '${eType}', got '${part.type}'`)
    arktest.assertEQ(part.value, eValue, `DTF range value mismatch: expected '${eValue}', got '${part.value}'`)
    arktest.assertEQ(part.source, eSource, `DTF range source mismatch: expected '${eSource}', got '${part.source}'`)
}

function assertNfRangePart(part: Intl.NumberRangeFormatPart, eType: string, eValue: string,
                           eSource: string): void {
    arktest.assertEQ(part.type, eType, `NF range type mismatch: expected '${eType}', got '${part.type}'`)
    arktest.assertEQ(part.value, eValue, `NF range value mismatch: expected '${eValue}', got '${part.value}'`)
    arktest.assertEQ(part.source, eSource, `NF range source mismatch: expected '${eSource}', got '${part.source}'`)
}

// --- DateTimeFormat.formatRangeToParts tests ---

// Basic date range: en-US y/m/d — verifies full part structure with source attribution
function testDtfRangeBasicDate(): void {
    let dtf = new Intl.DateTimeFormat('en-US', {
        year: 'numeric',
        month: '2-digit',
        day: '2-digit',
        timeZone: 'UTC'
    })
    let start = new Date(Date.UTC(2024, 0, 15)) // Jan 15, 2024
    let end = new Date(Date.UTC(2024, 5, 20))   // Jun 20, 2024
    let parts = dtf.formatRangeToParts(start, end)

    arktest.assertTrue(parts !== null, 'should produce parts')
    arktest.assertTrue(parts.length >= 7, 'date range should have at least 7 parts (3+1+3)')

    // Expected: 01/15/2024 – 06/20/2024
    // startRange parts
    assertDtfRangePart(parts[0], 'month', '1', 'startRange')
    assertDtfRangePart(parts[1], 'literal', '/', 'startRange')
    assertDtfRangePart(parts[2], 'day', '15', 'startRange')
    assertDtfRangePart(parts[3], 'literal', '/', 'startRange')
    assertDtfRangePart(parts[4], 'year', '2024', 'startRange')

    // shared separator between ranges
    assertDtfRangePart(parts[5], 'literal', ' \u2013 ', 'shared') // ' – ' (en-dash with spaces)

    // endRange parts
    assertDtfRangePart(parts[6], 'month', '6', 'endRange')
    assertDtfRangePart(parts[7], 'literal', '/', 'endRange')
    assertDtfRangePart(parts[8], 'day', '20', 'endRange')
    assertDtfRangePart(parts[9], 'literal', '/', 'endRange')
    assertDtfRangePart(parts[10], 'year', '2024', 'endRange')
}

// Time-only range: verifies hour/minute source attribution
function testDtfRangeTimeOnly(): void {
    let dtf = new Intl.DateTimeFormat('en-US', {
        hour: '2-digit',
        minute: '2-digit',
        timeZone: 'UTC'
    })
    let start = new Date(Date.UTC(2024, 0, 1, 9, 30, 0))  // 09:30
    let end = new Date(Date.UTC(2024, 0, 1, 14, 45, 0))   // 14:45
    let parts = dtf.formatRangeToParts(start, end)

    arktest.assertTrue(parts !== null, 'should produce time range parts')

    // Verify that each part has a valid source
    let hasStart = false
    let hasEnd = false
    let hasShared = false
    for (let part of parts) {
        if (part.source == 'startRange') { hasStart = true }
        if (part.source == 'endRange') { hasEnd = true }
    }
    arktest.assertTrue(hasStart, 'should have startRange parts')
    arktest.assertTrue(hasEnd, 'should have endRange parts')

    // Find the shared separator
    for (let part of parts) {
        if (part.source == 'shared') {
            hasShared = true
            arktest.assertEQ(part.type, 'literal', 'shared part should be a literal separator')
            break
        }
    }
    arktest.assertTrue(hasShared, 'should have at least one shared literal separator')
}

// Same start and end date: verifies behavior when range collapses
function testDtfRangeSameDate(): void {
    let dtf = new Intl.DateTimeFormat('en-US', {
        year: 'numeric',
        month: '2-digit',
        day: '2-digit',
        timeZone: 'UTC'
    })
    let date = new Date(Date.UTC(2024, 2, 15)) // Mar 15, 2024
    let parts = dtf.formatRangeToParts(date, date)

    arktest.assertTrue(parts !== null, 'same-date range should produce parts')
    // When start==end, ICU may collapse or produce identical start/end
    // Verify all parts have a valid source
    for (let part of parts) {
        arktest.assertTrue(
            part.source == 'startRange' || part.source == 'endRange' || part.source == 'shared',
            `unexpected source '${part.source}' for same-date range`
        )
    }
}

// Source attribution: verify in-range separators get range source, between-range gets shared
function testDtfRangeSourceAttribution(): void {
    let dtf = new Intl.DateTimeFormat('en-US', {
        year: 'numeric',
        month: '2-digit',
        day: '2-digit',
        timeZone: 'UTC'
    })
    let start = new Date(Date.UTC(2024, 0, 15))
    let end = new Date(Date.UTC(2024, 5, 20))
    let parts = dtf.formatRangeToParts(start, end)

    // The "/" separators within each date should inherit the range's source,
    // while the " – " separator between ranges should be "shared".
    let foundSharedLiteral = false
    for (let part of parts) {
        if (part.source == 'shared') {
            foundSharedLiteral = true
            // Shared parts should only be literals (separators between ranges)
            arktest.assertEQ(part.type, 'literal',
                'shared-source parts should be literal separators only')
        }
    }
    arktest.assertTrue(foundSharedLiteral, 'should have at least one shared literal between ranges')

    // In-range literals should NOT be shared
    for (let part of parts) {
        if (part.source == 'startRange' && part.type == 'literal') {
            arktest.assertEQ(part.value, '/',
                'startRange literal should be the date separator \'/\'')
        }
        if (part.source == 'endRange' && part.type == 'literal') {
            arktest.assertEQ(part.value, '/',
                'endRange literal should be the date separator \'/\'')
        }
    }
}

// Different years: verify both ranges show correct year values
function testDtfRangeDifferentYears(): void {
    let dtf = new Intl.DateTimeFormat('en-US', {
        year: 'numeric',
        month: 'short',
        timeZone: 'UTC'
    })
    let start = new Date(Date.UTC(2020, 0, 1))  // Jan 2020
    let end = new Date(Date.UTC(2025, 6, 1))    // Jul 2025
    let parts = dtf.formatRangeToParts(start, end)

    arktest.assertTrue(parts !== null, 'different-year range should produce parts')

    // Find year parts and verify they have correct values and sources
    let foundStartYear = false
    let foundEndYear = false
    for (let part of parts) {
        if (part.type == 'year') {
            if (part.source == 'startRange') {
                arktest.assertEQ(part.value, '2020', 'start year should be 2020')
                foundStartYear = true
            } else if (part.source == 'endRange') {
                arktest.assertEQ(part.value, '2025', 'end year should be 2025')
                foundEndYear = true
            }
        }
    }
    arktest.assertTrue(foundStartYear, 'should have startRange year part')
    arktest.assertTrue(foundEndYear, 'should have endRange year part')
}

// --- NumberFormat.formatRangeToParts tests ---

// Basic decimal range: 1 – 5
function testNfRangeBasic(): void {
    let nf = new Intl.NumberFormat('en')
    let parts = nf.formatRangeToParts(1, 5)

    arktest.assertTrue(parts !== null, 'should produce range parts')
    arktest.assertTrue(parts.length >= 3, 'basic range should have at least 3 parts')

    // Expected: integer:"1":startRange, literal:"–":shared, integer:"5":endRange
    assertNfRangePart(parts[0], 'integer', '1', 'startRange')
    assertNfRangePart(parts[1], 'literal', '\u2013', 'shared') // en-dash without spaces for plain decimal
    assertNfRangePart(parts[2], 'integer', '5', 'endRange')
}

// Equal values collapse to approximatelySign + shared integer
// "approximatelySign" is not in NumberFormatPartTypes union but IS emitted at runtime
// by native code (IntlNumberFormat.cpp:256, UNUM_APPROXIMATELY_SIGN_FIELD) when equal
// values are collapsed by ICU. Use assertNfRangePart (takes string parameter) to avoid
// type-narrowing making an inline literal comparison always false.
function testNfRangeSameValue(): void {
    let nf = new Intl.NumberFormat('en')
    let parts = nf.formatRangeToParts(10, 10)

    arktest.assertTrue(parts !== null, 'equal range should produce parts')
    arktest.assertTrue(parts.length >= 2, 'collapsed range should have at least 2 parts')

    assertNfRangePart(parts[0], 'approximatelySign', '~', 'shared')
    assertNfRangePart(parts[1], 'integer', '10', 'shared')
}

// Negative range: -10 – -5
function testNfRangeNegative(): void {
    let nf = new Intl.NumberFormat('en')
    let parts = nf.formatRangeToParts(-10, -5)

    arktest.assertTrue(parts !== null, 'negative range should produce parts')

    // Verify minusSign parts exist for both ranges
    let startHasMinus = false
    let endHasMinus = false
    for (let part of parts) {
        if (part.type == 'minusSign') {
            if (part.source == 'startRange') {
                startHasMinus = true
            } else if (part.source == 'endRange') {
                endHasMinus = true
            }
        }
    }
    arktest.assertTrue(startHasMinus, 'startRange should have minusSign')
    arktest.assertTrue(endHasMinus, 'endRange should have minusSign')
}

// Fractional range: verify decimal/fraction parts within each range
function testNfRangeFractional(): void {
    let nf = new Intl.NumberFormat('en', { minimumFractionDigits: 1, maximumFractionDigits: 1 })
    let parts = nf.formatRangeToParts(1.5, 2.5)

    arktest.assertTrue(parts !== null, 'fractional range should produce parts')

    // Verify each range has integer + decimal + fraction parts
    let startInteger = false
    let startFraction = false
    let endInteger = false
    let endFraction = false
    for (let part of parts) {
        if (part.source == 'startRange') {
            if (part.type == 'integer') { startInteger = true }
            if (part.type == 'fraction') { startFraction = true }
        }
        if (part.source == 'endRange') {
            if (part.type == 'integer') { endInteger = true }
            if (part.type == 'fraction') { endFraction = true }
        }
    }
    arktest.assertTrue(startInteger, 'startRange should have integer part')
    arktest.assertTrue(startFraction, 'startRange should have fraction part')
    arktest.assertTrue(endInteger, 'endRange should have integer part')
    arktest.assertTrue(endFraction, 'endRange should have fraction part')
}

// All parts should have valid source values
function testNfRangeAllSourcesValid(): void {
    let nf = new Intl.NumberFormat('en')
    let parts = nf.formatRangeToParts(3, 7)

    for (let part of parts) {
        arktest.assertTrue(
            part.source == 'startRange' || part.source == 'endRange' || part.source == 'shared',
            `invalid source '${part.source}' for type '${part.type}'`
        )
    }
}

function main(): int {
    const suite = new arktest.ArkTestsuite('formatRangeToParts tests')

    // DateTimeFormat.formatRangeToParts
    suite.addTest('DTF range basic date', testDtfRangeBasicDate)
    suite.addTest('DTF range time only', testDtfRangeTimeOnly)
    suite.addTest('DTF range same date', testDtfRangeSameDate)
    suite.addTest('DTF range source attribution', testDtfRangeSourceAttribution)
    suite.addTest('DTF range different years', testDtfRangeDifferentYears)

    // NumberFormat.formatRangeToParts
    suite.addTest('NF range basic', testNfRangeBasic)
    suite.addTest('NF range same value (collapse)', testNfRangeSameValue)
    suite.addTest('NF range negative', testNfRangeNegative)
    suite.addTest('NF range fractional', testNfRangeFractional)
    suite.addTest('NF range all sources valid', testNfRangeAllSourcesValid)

    return suite.run()
}
