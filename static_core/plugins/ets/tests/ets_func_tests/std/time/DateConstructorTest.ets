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
    let failures = 0;

    failures += test(testIntMsConstructor(), "Test MS constructor with Long");
    failures += test(testIntYearMonthConstructor(), "Test year and month constructor with Long");
    failures += test(testIntYearMonthDayConstructor(), "Test year, month and day constructor with Long");
    failures += test(testIntYearMonthDayHoursConstructor(), "Test YMDH constructor with Long");
    failures += test(testIntYearMonthDayHoursMinutesConstructor(),"Test YMDH and minutes constructor with Long");
    failures += test(testIntYearMonthDayHoursMinutesSecondsConstructor(),"Test YMDH minutes and seconds constructor with Long");
    failures += test(testIntYearMonthDayHoursMinutesSecondsMillisConstructor(),"Test YMDH minutes seconds and millis constructor with Long");

    failures += test(testDoubleMsConstructor(), "Test MS constructor with Double");
    failures += test(testDoubleYearMonthConstructor(), "Test year and month constructor with Double");
    failures += test(testDoubleYearMonthDayConstructor(), "Test year, month and day constructor with Double");
    failures += test(testDoubleYearMonthDayHoursConstructor(),"Test YMDH constructor with Double")
    failures += test(testDoubleYearMonthDayHoursMinutesConstructor(),"Test YMDH and minutes constructor with Double");
    failures += test(testDoubleYearMonthDayHoursMinutesSecondsConstructor(),"Test YMDH minutes and seconds constructor with Double");
    failures += test(testDoubleYearMonthDayHoursMinutesSecondsMillisConstructor(),"Test YMDH minutes seconds and millis constructor with Double");

    failures += test(testStringConstructor(), "Test constructor with string")
    failures += test(testDateConstructor(), "Test constructor with Date object")
    failures += test(testNaNConstructor(), "Test constructor with NaN")
    
    return test(failures, "All tests run");
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

function testIntMsConstructor(): int {

    let failures = 0;

    for(let i: int = 0; i < longYears.length; i++) {

        let cd = new Date(longYears[i]);
        
        try {
            if(cd.valueOf() as long != longYears[i]) failures++;
        } catch (e) {
            return 1;
        }
        
    }

    return failures;
}

function testIntYearMonthConstructor(): int {
    let md: long[] = longMonths;
    let failures = 0;

    for(let i: int = 0; i < longYears.length; i++) {
        let cd = new Date(longYears[i], md[i]);
        let utc = Date.UTC(longYears[i], md[i]);
        try {
            if(utc as long != (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long) failures++;
        } catch (e) {
            return 1;
        }
        failures += checkDateWithLongYMD(cd, valueLongYears[i], md[i], 1);
    }

    return failures;
}

function testIntYearMonthDayConstructor(): int {

    let md: long[] = longMonths;
    let failures = 0;

    for(let i: int = 0; i < longYears.length; i++) {
        let day: long = md[i] + 1;
        let cd = new Date(longYears[i], md[i], day);
        let utc = Date.UTC(longYears[i], md[i], day);
        try {
            if(utc as long != (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long) failures++;
        } catch (e) {
            return 1;
        }
        failures += checkDateWithLongYMD(cd, valueLongYears[i], md[i], day);
    }

    return failures;
}

function testIntYearMonthDayHoursConstructor(): int {

    let md: long[] = longMonths;
    let ht: long[] = longHours;
    let failures = 0;

    for(let i: int = 0; i < longYears.length; i++) {
        let day: long = md[i] + 1;
        let cd = new Date(longYears[i], md[i], day, ht[i]);
        let utc = Date.UTC(longYears[i], md[i], day, ht[i]);
        try {
            if(utc as long != (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long) failures++;
        } catch (e) {
            return 1;
        }
        failures += checkDateWithLongYMDH(cd, valueLongYears[i], md[i], day, ht[i]);
    }

    return failures;
}

function testIntYearMonthDayHoursMinutesConstructor(): int {

    let md: long[] = longMonths;
    let ht: long[] = longHours;
    let mh: long[] = longMinutes;
    let failures = 0;

    for(let i: int = 0; i < longYears.length; i++) {
        let day: long = md[i] + 1;
        let cd = new Date(longYears[i], md[i], day, ht[i], mh[i]);
        let utc = Date.UTC(longYears[i], md[i], day, ht[i], mh[i]);
        try {
            if(utc as long != (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long) failures++;
        } catch (e) {
            return 1;
        }
        failures += checkDateWithLongYMDHM(cd, valueLongYears[i], md[i], day, ht[i], mh[i]);
    }

    return failures;
}

function testIntYearMonthDayHoursMinutesSecondsConstructor(): int {

    let md: long[] = longMonths;
    let ht: long[] = longHours;
    let mh: long[] = longMinutes;

    let failures = 0;

    for(let i: int = 0; i < longYears.length; i++) {
        let day: long = md[i] + 1;
        let cd = new Date(longYears[i], md[i], day, ht[i], mh[i], mh[i]);
        let utc = Date.UTC(longYears[i], md[i], day, ht[i], mh[i], mh[i]);
        try {
            if(utc as long != (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long) failures++;
        } catch (e) {
            return 1;
        }
        failures += checkDateWithLongYMDHMS(cd, valueLongYears[i], md[i], day, ht[i], mh[i], mh[i]);
    }

    return failures;
}

function testIntYearMonthDayHoursMinutesSecondsMillisConstructor(): int {

    let md: long[] = longMonths;
    let ht: long[] = longHours;
    let mh: long[] = longMinutes
    let failures = 0;

    for(let i: int = 0; i < longYears.length; i++) {
        let day: long = md[i] + 1;
        let millis: long = mh[i]*10;

        let cd = new Date(longYears[i], md[i], day, ht[i], mh[i], mh[i], millis);
        let utc = Date.UTC(longYears[i], md[i], day, ht[i], mh[i], mh[i], millis);
        try {
            if(utc as long != (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long) failures++;
        } catch (e) {
            return 1;
        }
        failures += checkDateWithLongYMDHMSM(cd, valueLongYears[i], md[i], day, ht[i], mh[i], mh[i], millis);
    }

    return failures;
}

function testDoubleMsConstructor(): int {
    let id: double[] = doubleYears;
    let failures = 0;

    for(let i: int = 0; i < id.length; i++) {
        let cd = new Date(id[i]);
        
        try {
            if(Double.valueOf(doubleYears[i]).intValue() != cd.valueOf()) failures++;
        } catch (e) {
            return 1;
        }

    }

    return failures;
}

function testDoubleYearMonthConstructor(): int {
    let id: double[] = doubleYears;
    let md: double[] = doubleMonths;
    let failures = 0;

    for(let i: int = 0; i < id.length; i++) {
        let cd = new Date(id[i], md[i]);
        let utc = Date.UTC(id[i], md[i]);
        try {
            if(utc as long != (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long) failures++;
        } catch (e) {
            return 1;
        }
        failures += checkDateWithDoubleYMD(cd, valueDoubleYears[i], md[i], 1);
    }

    return failures;
}

function testDoubleYearMonthDayConstructor(): int {
    let id: double[] = doubleYears;
    let md: double[] = doubleMonths;
    let failures = 0;

    for(let i: int = 0; i < id.length; i++) {
        let day: double = md[i] + 1;
        let cd = new Date(id[i], md[i], day);
        let utc = Date.UTC(id[i], md[i], new Double(day));
        try {
            if(utc as long != (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long) failures++;
        } catch (e) {
            return 1;
        }
        failures += checkDateWithDoubleYMD(cd, valueDoubleYears[i], md[i], day);
    }

    return failures;
}

function testDoubleYearMonthDayHoursConstructor(): int {
    let id: double[] = doubleYears;
    let md: double[] = doubleMonths;
    let ht: double[] = doubleHours;
    let failures = 0;

    for(let i: int = 0; i < id.length; i++) {
        let day: double = md[i] + 1;
        let cd = new Date(id[i], md[i], day, ht[i]);
        let utc = Date.UTC(id[i], md[i], new Double(day), new Double(ht[i]));
        try {
            if(utc as long != (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long) failures++;
        } catch (e) {
            return 1;
        }
        failures += checkDateWithDoubleYMDH(cd, valueDoubleYears[i], md[i], day, ht[i]);
    }

    return failures;
}

function testDoubleYearMonthDayHoursMinutesConstructor(): int {
    let id: double[] = doubleYears;
    let md: double[] = doubleMonths;
    let ht: double[] = doubleHours;
    let mh: double[] = doubleMinutes;
    let failures = 0;

    for(let i: int = 0; i < id.length; i++) {

        let day: double = md[i] + 1;
        let cd = new Date(id[i], md[i], day, ht[i], mh[i]);
        let utc = Date.UTC(id[i], md[i], new Double(day), new Double(ht[i]), new Double(mh[i]));
        try {
            if(utc as long != (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long) failures++;
        } catch (e) {
            return 1;
        }
        failures += checkDateWithDoubleYMDHM(cd, valueDoubleYears[i], md[i], day, ht[i], mh[i]);
    }

    return failures;
}

function testDoubleYearMonthDayHoursMinutesSecondsConstructor(): int {
    let id: double[] = doubleYears;
    let md: double[] = doubleMonths;
    let ht: double[] = doubleHours;
    let mh: double[] = doubleMinutes;
    let failures = 0;

    for(let i: int = 0; i < id.length; i++) {

        let day: double = md[i] + 1;

        let cd = new Date(id[i], md[i], day, ht[i], mh[i], mh[i]);
        let utc = Date.UTC(id[i], md[i], new Double(day), new Double(ht[i]), new Double(mh[i]), new Double(mh[i]));
        try {
            if(utc as long != (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long) failures++;
        } catch (e) {
            return 1;
        }
        failures += checkDateWithDoubleYMDHMS(cd, valueDoubleYears[i], md[i], day, ht[i], mh[i], mh[i]);
    }

    return failures;
}

function testDoubleYearMonthDayHoursMinutesSecondsMillisConstructor(): int {
    let id: double[] = doubleYears;
    let md: double[] = doubleMonths;
    let ht: double[] = doubleHours;
    let mh: double[] = doubleMinutes;
    let failures = 0;

    for(let i: int = 0; i < id.length; i++) {

        let day: double = md[i] + 1;
        let millis: double = mh[i]*10;

        let cd = new Date(id[i], md[i], day, ht[i], mh[i], mh[i], millis);
        let utc = Date.UTC(id[i], md[i], new Double(day), new Double(ht[i]), new Double(mh[i]), new Double(mh[i]), new Double(millis));
        try {
            if(utc as long != (cd.valueOf() - cd.getTimezoneOffset() * msPerMinute) as long) failures++;
        } catch (e) {
            return 1;
        }
        failures += checkDateWithDoubleYMDHMSM(cd, valueDoubleYears[i], md[i], day, ht[i], mh[i], mh[i], millis);
    }

    return failures;
}

function testStringConstructor(): int {
    let failures = 0;
    let sd: string[] = dateString;

    for(let i: int = 0; i < sd.length; i++) {
        let cd: Date = new Date(sd[i]);
        try {
            if(sd[i] != cd.toISOString()) failures++;
        } catch (e) {
            return 1;
        }
    }
    return failures;
}

function testDateConstructor(): int {
    let id: double[] = doubleYears;
    let md: double[] = doubleMonths;
    let ht: double[] = doubleHours;
    let mh: double[] = doubleMinutes;
    let failures: int = 0;
    for(let i: int = 0; i < id.length; i++) {

        let day: double = md[i] + 1;
        let tmp = new Date(id[i], md[i], day, ht[i], mh[i], mh[i]);
        let cd = new Date(tmp);
        try {
            if(tmp.valueOf() != cd.valueOf()) failures++;
        } catch (e) {
            return 1;
        }
        failures += checkDateWithDoubleYMDHMS(tmp, valueDoubleYears[i], md[i], day, ht[i], mh[i], mh[i]);
    }

    return failures;
    
}

function testNaNConstructor(): int {
    try {
        let d1 = new Date(Number.NaN);
        let val = d1.valueOf();
        assertTrue(isNaN(val));
    } catch (e) {
        return 1;
    }

    return 0;
}

//*************************************************************************************************************************************************************

function checkDateWithLongYMD(date: Date, year: long, month: long, day: long): int {
    return  checkDateWithLongYMDH(date, year, month, day, 0);
}

function checkDateWithLongYMDH(date: Date, year: long, month: long, day: long, hour: long): int {
    return checkDateWithLongYMDHM(date, year, month, day, hour, 0);
}

function checkDateWithLongYMDHM(date: Date, year: long, month: long, day: long, hour: long, minutes: long): int {
    return checkDateWithLongYMDHMS(date, year, month, day, hour, minutes, 0);
}

function checkDateWithLongYMDHMS(date: Date, year: long, month: long, day: long, hour: long, minutes: long, seconds: long): int {
    return checkDateWithLongYMDHMSM(date, year, month, day, hour, minutes, seconds, 0);
}

function checkDateWithLongYMDHMSM(cd: Date, year: long, month: long, day: long, hour: long, minutes: long, seconds: long, millis: long): int {
    let failures = 0;


        if(cd.getFullYear() != year) {
            failures++
            //console.println("" + year + "<->" + cd.getFullYear())
        }
        if(cd.getMonth() != month) {
            failures++
            //console.println("" + month + "<->" + cd.getMonth())
        }
        if(cd.getDate() != day) {
            failures++;
            //console.println("Day mismatch for " + cd + " " + cd.getDate() + " vs: " + day)
        }
        if(cd.getHours() != hour) {
            failures++;
            //console.println("Hours mismatch for " + cd + " " + cd.getHours() + " vs: " + hour)
        }
        if(cd.getMinutes() != minutes) failures++;
        if(cd.getSeconds() != seconds) failures++;
        if(cd.getMilliseconds() != millis) failures++;

    if(failures > 0) return 1;
    return 0;
}

//*************************************************************************************************************************************************************

function checkDateWithDoubleYMD(date: Date, year: double, month: double, day: double): int {
    return  checkDateWithDoubleYMDH(date, year, month, day, 0 as double);
}

function checkDateWithDoubleYMDH(date: Date, year: double, month: double, day: double, hour: double): int {
    return checkDateWithDoubleYMDHM(date, year, month, day, hour, 0 as double);
}

function checkDateWithDoubleYMDHM(date: Date, year: double, month: double, day: double, hour: double, minutes: double): int {
    return checkDateWithDoubleYMDHMS(date, year, month, day, hour, minutes, 0 as double);
}

function checkDateWithDoubleYMDHMS(date: Date, year: double, month: double, day: double, hour: double, minutes: double, seconds: double): int {
    return checkDateWithDoubleYMDHMSM(date, year, month, day, hour, minutes, seconds, 0 as double);
}

function checkDateWithDoubleYMDHMSM(cd: Date, year: double, month: double, day: double, hour: double, minutes: double, seconds: double, millis: double): int {
    let failures = 0;
        if(cd.getFullYear() != trunc(year)) {
            failures++
            //console.println("" + year + "<->" + cd.getFullYear())
        }
        if(cd.getMonth() != trunc(month)) {
            failures++
            //console.println("" + month + "<->" + cd.getMonth())
        }
        if(cd.getDate() != trunc(day)) {
            failures++;
            //console.println("Day mismatch for " + cd + " " + cd.getDate() + " vs: " + day)
        }
        if(cd.getHours() != trunc(hour)) {
            failures++;
            //console.println("Hours mismatch for " + cd + " " + cd.getHours() + " vs: " + hour)
        }
        if(cd.getMinutes() != trunc(minutes)) failures++;
        if(cd.getSeconds() != trunc(seconds)) failures++;
        if(cd.getMilliseconds() != trunc(millis)) failures++;

    if(failures > 0) return 1;
    return 0;
}

