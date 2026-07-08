/*
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

function testSetUTCDayMovesWithinSameWeek() {
    let date = new Date(Date.UTC(2024, 0, 3, 12, 30, 15, 250));
    let result = date.setUTCDay(0);

    arktest.assertEQ(result, date.valueOf());
    arktest.assertEQ(date.toUTCString(), 'Sun, 31 Dec 2023 12:30:15 GMT');
    arktest.assertEQ(date.getUTCDay(), 0);
    arktest.assertEQ(date.getUTCHours(), 12);
    arktest.assertEQ(date.getUTCMinutes(), 30);
    arktest.assertEQ(date.getUTCSeconds(), 15);
    arktest.assertEQ(date.getUTCMilliseconds(), 250);
}

function testSetUTCDayMovesForwardWithinSameWeek() {
    let date = new Date(Date.UTC(2024, 0, 3, 12, 30, 15, 250));
    date.setUTCDay(6);

    arktest.assertEQ(date.toUTCString(), 'Sat, 06 Jan 2024 12:30:15 GMT');
    arktest.assertEQ(date.getUTCDay(), 6);
}

function testSetUTCDayKeepsDateWhenSameDay() {
    let date = new Date(Date.UTC(2024, 1, 29, 8, 9, 10, 11));
    let result = date.setUTCDay(4);

    arktest.assertEQ(result, date.valueOf());
    arktest.assertEQ(date.toUTCString(), 'Thu, 29 Feb 2024 08:09:10 GMT');
    arktest.assertEQ(date.getUTCDay(), 4);
    arktest.assertEQ(date.getUTCMilliseconds(), 11);
}

function testSetUTCDayMovesBackwardAcrossMonth() {
    let date = new Date(Date.UTC(2024, 2, 1, 0, 0, 0, 0));
    date.setUTCDay(1);

    arktest.assertEQ(date.toUTCString(), 'Mon, 26 Feb 2024 00:00:00 GMT');
    arktest.assertEQ(date.getUTCDay(), 1);
    arktest.assertEQ(date.getUTCDate(), 26);
}

function testSetUTCDayMovesForwardAcrossYear() {
    let date = new Date(Date.UTC(2023, 11, 31, 23, 59, 59, 999));
    date.setUTCDay(6);

    arktest.assertEQ(date.toUTCString(), 'Sat, 06 Jan 2024 23:59:59 GMT');
    arktest.assertEQ(date.getUTCDay(), 6);
    arktest.assertEQ(date.getUTCMilliseconds(), 999);
}

function testSetUTCDayAcceptsNegativeValues() {
    let date = new Date(Date.UTC(2024, 0, 3));
    let result = date.setUTCDay(-1);

    arktest.assertEQ(result, date.valueOf());
    arktest.assertEQ(date.toUTCString(), 'Sat, 30 Dec 2023 00:00:00 GMT');
    arktest.assertEQ(date.getUTCDay(), 6);
}

function testSetUTCDayAcceptsValuesAboveWeekRange() {
    let date = new Date(Date.UTC(2024, 0, 3));
    let result = date.setUTCDay(7);

    arktest.assertEQ(result, date.valueOf());
    arktest.assertEQ(date.toUTCString(), 'Sun, 07 Jan 2024 00:00:00 GMT');
    arktest.assertEQ(date.getUTCDay(), 0);
}

function testSetUTCDayAcceptsLargeValues() {
    let date = new Date(Date.UTC(2024, 0, 3, 12, 30, 15, 250));
    let result = date.setUTCDay(15);

    arktest.assertEQ(result, date.valueOf());
    arktest.assertEQ(date.toUTCString(), 'Mon, 15 Jan 2024 12:30:15 GMT');
    arktest.assertEQ(date.getUTCDay(), 1);
    arktest.assertEQ(date.getUTCMilliseconds(), 250);
}

function main(): int {
    let myTestsuite = new arktest.ArkTestsuite('DateSetUTCDayTest');
    myTestsuite.addTest('testSetUTCDayMovesWithinSameWeek', testSetUTCDayMovesWithinSameWeek);
    myTestsuite.addTest('testSetUTCDayMovesForwardWithinSameWeek', testSetUTCDayMovesForwardWithinSameWeek);
    myTestsuite.addTest('testSetUTCDayKeepsDateWhenSameDay', testSetUTCDayKeepsDateWhenSameDay);
    myTestsuite.addTest('testSetUTCDayMovesBackwardAcrossMonth', testSetUTCDayMovesBackwardAcrossMonth);
    myTestsuite.addTest('testSetUTCDayMovesForwardAcrossYear', testSetUTCDayMovesForwardAcrossYear);
    myTestsuite.addTest('testSetUTCDayAcceptsNegativeValues', testSetUTCDayAcceptsNegativeValues);
    myTestsuite.addTest('testSetUTCDayAcceptsValuesAboveWeekRange', testSetUTCDayAcceptsValuesAboveWeekRange);
    myTestsuite.addTest('testSetUTCDayAcceptsLargeValues', testSetUTCDayAcceptsLargeValues);
    return myTestsuite.run();
}
