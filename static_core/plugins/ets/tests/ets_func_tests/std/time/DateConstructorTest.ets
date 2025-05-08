/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
    const suite = new ArkTestsuite("Date")

    suite.addTest("Test MS constructor with Long", testIntMsConstructor)
    suite.addTest("Test year and month constructor with Long", testIntYearMonthConstructor)
    suite.addTest("Test year, month and day constructor with Long", testIntYearMonthDayConstructor)
    suite.addTest("Test YMDH constructor with Long", testIntYearMonthDayHoursConstructor)
    suite.addTest("Test YMDH and minutes constructor with Long", testIntYearMonthDayHoursMinutesConstructor)
    suite.addTest("Test YMDH minutes and seconds constructor with Long", testIntYearMonthDayHoursMinutesSecondsConstructor)
    suite.addTest("Test YMDH minutes seconds and millis constructor with Long", testIntYearMonthDayHoursMinutesSecondsMillisConstructor)
    suite.addTest("Test MS constructor with Double", testDoubleMsConstructor)
    suite.addTest("Test year and month constructor with Double", testDoubleYearMonthConstructor)
    suite.addTest("Test year, month and day constructor with Double", testDoubleYearMonthDayConstructor);
    suite.addTest("Test YMDH constructor with Double", testDoubleYearMonthDayHoursConstructor)
    suite.addTest("Test YMDH and minutes constructor with Double", testDoubleYearMonthDayHoursMinutesConstructor)

    suite.addTest("Test YMDH minutes and seconds constructor with Double", testDoubleYearMonthDayHoursMinutesSecondsConstructor);
    suite.addTest("Test YMDH minutes seconds and millis constructor with Double", testDoubleYearMonthDayHoursMinutesSecondsMillisConstructor);

    suite.addTest("Test constructor with string", testStringConstructor)
    suite.addTest("Test constructor with Date object", testDateConstructor)
    suite.addTest("Test constructor with NaN", testNaNConstructor)
    suite.addTest("Test constructor with string (local TZ)", testStringConstructorLocalTZ)
    suite.addTest("Test constructor with string (invalid arguments)", testStringConstructorInvalidArgs)
    suite.addTest("Test constructor with string (expanded year)", testStringConstructorExpandedYear)

    return suite.run()
}

function test(result: int, message: String ): int {
    if(result == 0) {
      return 0;
    }
    console.println("FAILED: " + message);
    return 1;
}

const msPerMinute: long = 60000;
const longYears: long[] = [-300, -72, 1, 0, 99, 100, 1900, 1969, 1970, 1971, 1980, 2000, 2020, 2023, 2024, 2030, 2038, 2039];
const longMonths: long[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5];
const longHours: long[] =  [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 15, 17, 18, 19, 20, 21];
const longMinutes: long[] = [0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 0, 4, 8];
const valueLongYears: long[] = [-300, -72, 1901, 1900, 1999, 100, 1900, 1969, 1970, 1971, 1980, 2000, 2020, 2023, 2024, 2030, 2038, 2039];

const doubleYears: double[] = [-300, -72.0, 1.0, 0.0, 99.9, 100.0, 1900.5, 1969.9, 1970.0, 1971.1, 1980.0, 2000.0, 2020.20, 2023.9, 2024.6, 2030.3, 2038.5, 2039.9];
const doubleMonths: double[] = [0.0, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.10, 11.11, 0.2, 1.1, 2.2, 3.3, 4.4, 5.5];
const doubleHours: double[] = [0.0, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.10, 11.11, 15.5, 17.7, 20.22, 21.33, 22.22, 23.99];
const doubleMinutes: double[] = [0.0, 4.4, 8.8, 12.12, 16.16, 20.20, 24.24, 28.28, 32.32, 36.36, 40.40, 44.44, 48.48, 52.52, 56.56, 0.0, 4.4, 8.8];
const valueDoubleYears: double[] = [-300, -72.0, 1901.0, 1900.0, 1999.9, 100.3, 1900.5, 1969.9, 1970.0, 1971.1, 1980.0, 2000.0, 2020.20, 2023.9, 2024.6, 2030.3, 2038.5, 2039.9];

const dateString: string[] = ["2019-11-14T00:55:31.820Z", "2024-02-29T00:55:31.950Z", "2000-01-01T00:00:00.001Z"]

function testIntMsConstructor(): void {
    for(let i: int = 0; i < longYears.length; i++) {
        let cd = new Date(longYears[i]);
        assertEQ(cd.valueOf() as long, longYears[i])
    }
}

function testIntYearMonthConstructor(): void {
    for(let i: int = 0; i < longYears.length; i++) {
        let cd = new Date(longYears[i], longMonths[i]);
        let utc = Date.UTC(longYears[i], longMonths[i]);

        assertEQ(utc as long, (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long)
        checkDateWithLongYMD(cd, valueLongYears[i], longMonths[i], 1)
    }
}

function testIntYearMonthDayConstructor(): void {
    for(let i: int = 0; i < longYears.length; i++) {
        let day: long = longMonths[i] + 1;
        let cd = new Date(longYears[i], longMonths[i], day);
        let utc = Date.UTC(longYears[i], longMonths[i], day);

        assertEQ(utc as long, (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long)
        checkDateWithLongYMD(cd, valueLongYears[i], longMonths[i], day);
    }
}

function testIntYearMonthDayHoursConstructor(): void {
    for(let i: int = 0; i < longYears.length; i++) {
        let day: long = longMonths[i] + 1;
        let cd = new Date(longYears[i], longMonths[i], day, longHours[i]);
        let utc = Date.UTC(longYears[i], longMonths[i], day, longHours[i]);

        assertEQ(utc as long, (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long)
        checkDateWithLongYMDH(cd, valueLongYears[i], longMonths[i], day, longHours[i]);
    }
}

function testIntYearMonthDayHoursMinutesConstructor(): void {
    for(let i: int = 0; i < longYears.length; i++) {
        let day: long = longMonths[i] + 1;
        let cd = new Date(longYears[i], longMonths[i], day, longHours[i], longMinutes[i]);
        let utc = Date.UTC(longYears[i], longMonths[i], day, longHours[i], longMinutes[i]);

        assertEQ(utc as long, (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long)
        checkDateWithLongYMDHM(cd, valueLongYears[i], longMonths[i], day, longHours[i], longMinutes[i]);
    }
}

function testIntYearMonthDayHoursMinutesSecondsConstructor(): void {
    for(let i: int = 0; i < longYears.length; i++) {
        let day: long = longMonths[i] + 1;
        let cd = new Date(longYears[i], longMonths[i], day, longHours[i], longMinutes[i], longMinutes[i]);
        let utc = Date.UTC(longYears[i], longMonths[i], day, longHours[i], longMinutes[i], longMinutes[i]);

        assertEQ(utc as long, (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long)
        checkDateWithLongYMDHMS(cd, valueLongYears[i], longMonths[i], day, longHours[i], longMinutes[i], longMinutes[i]);
    }
}

function testIntYearMonthDayHoursMinutesSecondsMillisConstructor(): void {
    for(let i: int = 0; i < longYears.length; i++) {
        let day: long = longMonths[i] + 1;
        let millis: long = longMinutes[i] * 10;

        let cd = new Date(longYears[i], longMonths[i], day, longHours[i], longMinutes[i], longMinutes[i], millis);
        let utc = Date.UTC(longYears[i], longMonths[i], day, longHours[i], longMinutes[i], longMinutes[i], millis);

        assertEQ(utc as long, (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long)
        checkDateWithLongYMDHMSM(cd, valueLongYears[i], longMonths[i], day, longHours[i], longMinutes[i], longMinutes[i], millis);
    }
}

function testDoubleMsConstructor(): void {
    for(let i: int = 0; i < doubleYears.length; i++) {
        let cd = new Date(doubleYears[i]);
        assertEQ(Double.valueOf(doubleYears[i]).toInt(), cd.valueOf())
    }
}

function testDoubleYearMonthConstructor(): void {
    for(let i: int = 0; i < doubleYears.length; i++) {
        let cd = new Date(doubleYears[i], doubleMonths[i]);
        let utc = Date.UTC(doubleYears[i], doubleMonths[i]);

        assertEQ(utc as long, (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long)
        checkDateWithDoubleYMD(cd, valueDoubleYears[i], doubleMonths[i], 1);
    }
}

function testDoubleYearMonthDayConstructor(): void {
    for(let i: int = 0; i < doubleYears.length; i++) {
        let day: double = doubleMonths[i] + 1;
        let cd = new Date(doubleYears[i], doubleMonths[i], day);
        let utc = Date.UTC(doubleYears[i], doubleMonths[i], new Double(day));

        assertEQ(utc as long, (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long)
        checkDateWithDoubleYMD(cd, valueDoubleYears[i], doubleMonths[i], day);
    }
}

function testDoubleYearMonthDayHoursConstructor(): void {
    for(let i: int = 0; i < doubleYears.length; i++) {
        let day: double = doubleMonths[i] + 1;
        let cd = new Date(doubleYears[i], doubleMonths[i], day, doubleHours[i]);
        let utc = Date.UTC(doubleYears[i], doubleMonths[i], new Double(day), new Double(doubleHours[i]));

        assertEQ(utc as long, (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long)
        checkDateWithDoubleYMDH(cd, valueDoubleYears[i], doubleMonths[i], day, doubleHours[i]);
    }
}

function testDoubleYearMonthDayHoursMinutesConstructor(): void {
    for(let i: int = 0; i < doubleYears.length; i++) {
        let day: double = doubleMonths[i] + 1;
        let cd = new Date(doubleYears[i], doubleMonths[i], day, doubleHours[i], doubleMinutes[i]);
        let utc = Date.UTC(doubleYears[i], doubleMonths[i], new Double(day), new Double(doubleHours[i]), new Double(doubleMinutes[i]));

        assertEQ(utc as long, (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long)
        checkDateWithDoubleYMDHM(cd, valueDoubleYears[i], doubleMonths[i], day, doubleHours[i], doubleMinutes[i]);
    }
}

function testDoubleYearMonthDayHoursMinutesSecondsConstructor(): void {
    for(let i: int = 0; i < doubleYears.length; i++) {
        let day: double = doubleMonths[i] + 1;
        let cd = new Date(doubleYears[i], doubleMonths[i], day, doubleHours[i], doubleMinutes[i], doubleMinutes[i]);
        let utc = Date.UTC(doubleYears[i], doubleMonths[i], new Double(day), new Double(doubleHours[i]), new Double(doubleMinutes[i]), new Double(doubleMinutes[i]));

        assertEQ(utc as long, (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long)
        checkDateWithDoubleYMDHMS(cd, valueDoubleYears[i], doubleMonths[i], day, doubleHours[i], doubleMinutes[i], doubleMinutes[i]);
    }
}

function testDoubleYearMonthDayHoursMinutesSecondsMillisConstructor(): void {
    for(let i: int = 0; i < doubleYears.length; i++) {
        let day: double = doubleMonths[i] + 1;
        let millis: double = doubleMinutes[i] * 10;

        let cd = new Date(doubleYears[i], doubleMonths[i], day, doubleHours[i], doubleMinutes[i], doubleMinutes[i], millis);
        let utc = Date.UTC(doubleYears[i], doubleMonths[i], new Double(day),
                new Double(doubleHours[i]), new Double(doubleMinutes[i]), new Double(doubleMinutes[i]), new Double(millis));

        assertEQ(utc as long, (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long)
        checkDateWithDoubleYMDHMSM(cd, valueDoubleYears[i], doubleMonths[i], day, doubleHours[i], doubleMinutes[i], doubleMinutes[i], millis);
    }
}

function testStringConstructor(): void {
    for(let i: int = 0; i < dateString.length; i++) {
        let cd: Date = new Date(dateString[i]);
        assertEQ(dateString[i], cd.toISOString())
    }
}

function testDateConstructor(): void {
    for(let i: int = 0; i < doubleYears.length; i++) {
        let day: double = doubleMonths[i] + 1;
        let tmp = new Date(doubleYears[i], doubleMonths[i], day, doubleHours[i], doubleMinutes[i], doubleMinutes[i]);
        let cd = new Date(tmp);

        assertEQ(tmp.valueOf(), cd.valueOf())
        checkDateWithDoubleYMDHMS(tmp, valueDoubleYears[i], doubleMonths[i], day, doubleHours[i], doubleMinutes[i], doubleMinutes[i]);
    }
}

function testNaNConstructor(): void {
    let d1 = new Date(Number.NaN);
    let val = d1.valueOf();
    assertTrue(isNaN(val));
}

function testStringConstructorLocalTZ(): void {
    const localDateTime = new Date(2024, 1, 19, 8, 0, 0)
    const isoLocalDateTime = new Date("2024-02-19T08:00:00")
    assertEQ(localDateTime.getTime(), isoLocalDateTime.getTime())

    const isoLocalDate = new Date("1970-01-01")
    assertEQ(isoLocalDate.getTime(), 0)
}

function testStringConstructorInvalidArgs(): void {
    const startingSpaceDate = new Date(" 2024-02-19T08:00:00")
    assertTrue(isNaN(startingSpaceDate.valueOf()))
    assertTrue(isNaN(startingSpaceDate.getTime()))

    const trailingSpaceDate = new Date("2024-02-19T08:00:00 ")
    assertTrue(isNaN(trailingSpaceDate.valueOf()))
    assertTrue(isNaN(trailingSpaceDate.getTime()))

    const ms4Date = new Date("2024-02-28T00:55:31.1234Z")
    assertEQ(Date.UTC(2024, 1, 28, 0, 55, 31, 123), ms4Date.getTime())

    const ms2Date = new Date("2024-02-28T00:55:31.12Z")
    assertEQ(Date.UTC(2024, 1, 28, 0, 55, 31, 120), ms2Date.getTime())

    const missingMsDate = new Date("2024-02-28T00:55:31.")
    assertTrue(isNaN(missingMsDate.valueOf()))
    assertTrue(isNaN(missingMsDate.getTime()))
}

function testStringConstructorExpandedYear(): void {
    const date = new Date("-000001-07-01T00:00Z")
    assertEQ(date.toUTCString(), "Thu, 01 Jul -0001 00:00:00 GMT")
}

//*************************************************************************************************************************************************************

function checkDateWithLongYMD(date: Date, year: long, month: long, day: long): void {
    checkDateWithLongYMDH(date, year, month, day, 0);
}

function checkDateWithLongYMDH(date: Date, year: long, month: long, day: long, hour: long): void {
    checkDateWithLongYMDHM(date, year, month, day, hour, 0);
}

function checkDateWithLongYMDHM(date: Date, year: long, month: long, day: long, hour: long, minutes: long): void {
    checkDateWithLongYMDHMS(date, year, month, day, hour, minutes, 0);
}

function checkDateWithLongYMDHMS(date: Date, year: long, month: long, day: long, hour: long, minutes: long, seconds: long): void {
    checkDateWithLongYMDHMSM(date, year, month, day, hour, minutes, seconds, 0);
}

function checkDateWithLongYMDHMSM(cd: Date, year: long, month: long, day: long, hour: long, minutes: long, seconds: long, millis: long): void {
    assertEQ(cd.getFullYear(), year, "year mismatch")
    assertEQ(cd.getMonth(), month, "month mismatch")
    assertEQ(cd.getDate(), day, "day mismatch")
    assertEQ(cd.getHours(), hour, "hour mismatch")
    assertEQ(cd.getMinutes(), minutes, "minute mismatch")
    assertEQ(cd.getSeconds(), seconds, "second mismatch")
    assertEQ(cd.getMilliseconds(), millis, "ms mismatch")
}

//*************************************************************************************************************************************************************

function checkDateWithDoubleYMD(date: Date, year: double, month: double, day: double): void {
    checkDateWithDoubleYMDH(date, year, month, day, 0 as double);
}

function checkDateWithDoubleYMDH(date: Date, year: double, month: double, day: double, hour: double): void {
    checkDateWithDoubleYMDHM(date, year, month, day, hour, 0 as double);
}

function checkDateWithDoubleYMDHM(date: Date, year: double, month: double, day: double, hour: double, minutes: double): void {
    checkDateWithDoubleYMDHMS(date, year, month, day, hour, minutes, 0 as double);
}

function checkDateWithDoubleYMDHMS(date: Date, year: double, month: double, day: double, hour: double, minutes: double, seconds: double): void {
    checkDateWithDoubleYMDHMSM(date, year, month, day, hour, minutes, seconds, 0 as double);
}

function checkDateWithDoubleYMDHMSM(cd: Date, year: double, month: double, day: double, hour: double, minutes: double, seconds: double, millis: double): void {
    assertEQ(cd.getFullYear(), trunc(year), "truncated year mismatch")
    assertEQ(cd.getMonth(), trunc(month), "truncated month mismatch")
    assertEQ(cd.getDate(), trunc(day), "truncated day mismatch")
    assertEQ(cd.getHours(), trunc(hour), "truncated hour mismatch")
    assertEQ(cd.getMinutes(), trunc(minutes), "truncated minute mismatch")
    assertEQ(cd.getSeconds(), trunc(seconds), "truncated second mismatch")
    assertEQ(cd.getMilliseconds(), trunc(millis), "truncated ms mismatch")
}
