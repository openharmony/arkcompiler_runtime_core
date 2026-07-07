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

function testNormalYearPadding(): void {
    const date0 = new Date(Date.UTC(0, 0, 1))
    date0.setUTCFullYear(0)
    arktest.assertEQ(date0.toISOString(), '0000-01-01T00:00:00.000Z')

    const date1 = new Date(Date.UTC(1, 0, 1))
    date1.setUTCFullYear(1)
    arktest.assertEQ(date1.toISOString(), '0001-01-01T00:00:00.000Z')

    const date10 = new Date(Date.UTC(10, 0, 1))
    date10.setUTCFullYear(10)
    arktest.assertEQ(date10.toISOString(), '0010-01-01T00:00:00.000Z')

    const date99 = new Date(Date.UTC(99, 0, 1))
    date99.setUTCFullYear(99)
    arktest.assertEQ(date99.toISOString(), '0099-01-01T00:00:00.000Z')

    const date100 = new Date(Date.UTC(100, 0, 1))
    arktest.assertEQ(date100.toISOString(), '0100-01-01T00:00:00.000Z')

    const date1000 = new Date(Date.UTC(1000, 0, 1))
    arktest.assertEQ(date1000.toISOString(), '1000-01-01T00:00:00.000Z')

    const date9999 = new Date(Date.UTC(9999, 0, 1))
    arktest.assertEQ(date9999.toISOString(), '9999-01-01T00:00:00.000Z')
}

function testNegativeYear(): void {
    const date1 = new Date(Date.UTC(-1, 0, 1))
    arktest.assertEQ(date1.toISOString(), '-000001-01-01T00:00:00.000Z')

    const date2 = new Date(Date.UTC(-10, 0, 1))
    arktest.assertEQ(date2.toISOString(), '-000010-01-01T00:00:00.000Z')

    const date3 = new Date(Date.UTC(-100, 0, 1))
    arktest.assertEQ(date3.toISOString(), '-000100-01-01T00:00:00.000Z')

    const date4 = new Date(Date.UTC(-1000, 0, 1))
    arktest.assertEQ(date4.toISOString(), '-001000-01-01T00:00:00.000Z')

    const date5 = new Date(Date.UTC(-10000, 0, 1))
    arktest.assertEQ(date5.toISOString(), '-010000-01-01T00:00:00.000Z')

    const date6 = new Date(Date.UTC(-100000, 0, 1))
    arktest.assertEQ(date6.toISOString(), '-100000-01-01T00:00:00.000Z')
}

function testExpandedPositiveYear(): void {
    const date10000 = new Date(Date.UTC(10000, 0, 1))
    arktest.assertEQ(date10000.toISOString(), '+010000-01-01T00:00:00.000Z')

    const date100000 = new Date(Date.UTC(100000, 0, 1))
    arktest.assertEQ(date100000.toISOString(), '+100000-01-01T00:00:00.000Z')

    const date99999 = new Date(Date.UTC(99999, 0, 1))
    arktest.assertEQ(date99999.toISOString(), '+099999-01-01T00:00:00.000Z')

    const date275760 = new Date(Date.UTC(275760, 8, 13))
    arktest.assertEQ(date275760.toISOString(), '+275760-09-13T00:00:00.000Z')
}

function testBoundaryYear(): void {
    const date9999 = new Date(Date.UTC(9999, 11, 31, 23, 59, 59, 999))
    arktest.assertEQ(date9999.toISOString(), '9999-12-31T23:59:59.999Z')

    const date10000 = new Date(Date.UTC(10000, 0, 1))
    arktest.assertEQ(date10000.toISOString(), '+010000-01-01T00:00:00.000Z')
}

function testMonthDayPadding(): void {
    const date = new Date(Date.UTC(2024, 0, 1, 0, 0, 0, 0))
    arktest.assertEQ(date.toISOString(), '2024-01-01T00:00:00.000Z')

    const date2 = new Date(Date.UTC(2024, 9, 5, 5, 6, 7, 8))
    arktest.assertEQ(date2.toISOString(), '2024-10-05T05:06:07.008Z')

    const date3 = new Date(Date.UTC(2024, 11, 31, 23, 59, 59, 999))
    arktest.assertEQ(date3.toISOString(), '2024-12-31T23:59:59.999Z')
}

function testMillisecondPadding(): void {
    const date1 = new Date(Date.UTC(2024, 0, 1, 0, 0, 0, 1))
    arktest.assertEQ(date1.toISOString(), '2024-01-01T00:00:00.001Z')

    const date2 = new Date(Date.UTC(2024, 0, 1, 0, 0, 0, 10))
    arktest.assertEQ(date2.toISOString(), '2024-01-01T00:00:00.010Z')

    const date3 = new Date(Date.UTC(2024, 0, 1, 0, 0, 0, 99))
    arktest.assertEQ(date3.toISOString(), '2024-01-01T00:00:00.099Z')

    const date4 = new Date(Date.UTC(2024, 0, 1, 0, 0, 0, 100))
    arktest.assertEQ(date4.toISOString(), '2024-01-01T00:00:00.100Z')
}

function testEpoch(): void {
    const epoch = new Date(Date.UTC(1970, 0, 1))
    arktest.assertEQ(epoch.toISOString(), '1970-01-01T00:00:00.000Z')
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Date.toISOString')
    suite.addTest('Test normal year padding', testNormalYearPadding)
    suite.addTest('Test negative year', testNegativeYear)
    suite.addTest('Test expanded positive year', testExpandedPositiveYear)
    suite.addTest('Test boundary year 9999/10000', testBoundaryYear)
    suite.addTest('Test month day ms padding', testMonthDayPadding)
    suite.addTest('Test millisecond padding', testMillisecondPadding)
    suite.addTest('Test epoch', testEpoch)
    return suite.run()
}
