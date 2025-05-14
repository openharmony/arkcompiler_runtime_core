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

package escompat;

// TODO To offer protection against timing attacks and fingerprinting, the precision of `Date.getTime()`,
// `Date.now()` might get rounded depending on browser (target) settings.
// In Firefox, the `privacy.reduceTimerPrecision` preference is enabled by default
// and defaults to 20µs in Firefox 59; in 60 it will be 2ms.

/** Hours in a day. */
const hoursPerDay: int = 24;

/** Minutes in an hour. */
const minutesPerHour: int = 60;

/** Seconds in a minute. */
const secondsPerMinute: long = 60;

/** Milliseconds in a second. */
const msPerSecond: long = 1000;

/** msPerMinute == 60000 */
const msPerMinute: long = msPerSecond * secondsPerMinute;

/** msPerHour == 3600000 */
const msPerHour: long = msPerMinute * minutesPerHour;

/** msPerDay == 86400000 */
const msPerDay: long = msPerHour * hoursPerDay;

/** Days in a year */
const dayCountInYear: int = 365;

/** Days in a leap year */
const dayCountInLeapYear: int = 366;

/** Months in a year */
const monthCountPerYear: int = 12;

/** Max possible count of days in month */
const maxDaysInMonth = 31;

/**
 * This gives a range of 8,640,000,000,000,000 milliseconds
 * to either side of 01 January, 1970 UTC.
 */
const maxDateOffset: long = 8.64e15;

/** Day names */
const dayNames: String[] = [
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
];

/** Month names */
const monthNames: String[] = [
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
];

/** First day of a month in a year */
const firstDayInMonthNormal: int[] = [0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334];
/** First day of a month in a leap year */
const firstDayInMonthLeap: int[]   = [0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335];

/**
 * Returns first day with respect to year and month
 *
 * @param year
 *
 * @param month
 *
 * @returns first day
 */
function getFirstDayInMonth(year: int, month: int): int {
    assertTrue((year == 0 || year == 1), "Invalid value " + year + " for \"year\", has to be 0 or 1")
    assertTrue((month >= 0 && month <= 11), "Invalid value " + month + " for \"month\", has to be in range 0..11")
    if (year == 0) {
        return firstDayInMonthNormal[month];
    } else {
        return firstDayInMonthLeap[month];
    }
}

/**
 * Calculate the elapsed days since Unix Epoch.
 *
 * @returns elapsed days since Unix Epoch.
 */
function ecmaDayFromTime(time: long): int {
    if (time < 0) {
        time -= msPerDay - 1;
    }
    return (time / msPerDay) as int;
}

/**
 *  @see ECMA-262, 20.4.1.3
 *
 * @returns number of days in the given year.
 */
function ecmaDaysInYear(year: int): int {
    if ((year % 4 != 0) || ((year % 100 == 0) && (year % 400 != 0))) {
        return dayCountInYear;
    }

    return dayCountInLeapYear;
}

/**
 * @see ECMA-262, 21.4.1.3
 *
 * @returns Year, corresponding to the given time.
 */
function ecmaYearFromTime(time: long): int {
    let approx: int = floor(time / msPerDay / 365.2425) as int + 1970 as int;
    let yearMs: long = ecmaDayFromYear(approx) * msPerDay;

    if (yearMs > time) {
        approx--;
    }

    if ((yearMs + ecmaDaysInYear(approx) * msPerDay) <= time) {
        approx++;
    }

    return approx;
}

/**
 * @see ECMA-262, 21.4.1.4
 *
 * @returns Month number (within year).
 */
function ecmaMonthFromTime(time: long): byte {
    let year = ecmaYearFromTime(time);
    let dayWithinYear = ecmaDayFromTime(time) - ecmaDayFromYear(year);

    assertTrue(dayWithinYear >= 0 && dayWithinYear < dayCountInLeapYear,
        "Invalid value " + dayWithinYear + " for \"dayWithinYear\", has to be in range 0.." + dayCountInLeapYear);

    let inLeapYear = ecmaInLeapYear(year);

    for (let i = 1; i < 12; i++) {
        if (dayWithinYear < getFirstDayInMonth(inLeapYear, i)) {
            return (i - 1) as byte;
        }
    }

    return 11;
}

/**
 * @see ECMA-262, 21.4.1.5
 *
 * @returns Date number (within month).
 */
function ecmaDateFromTime(time: long): byte {
    let year = ecmaYearFromTime(time);
    let dayWithinYear = ecmaDayFromTime(time) - ecmaDayFromYear(year);

    assertTrue(dayWithinYear >= 0 && dayWithinYear < dayCountInLeapYear,
        "Invalid value " + dayWithinYear + " for dayWithinYear, has to be in range 0.." + dayCountInLeapYear);

    let inLeapYear = ecmaInLeapYear(year);

    let month: byte = 11;

    for (let i = 1; i < 12; i++) {
        if (dayWithinYear < getFirstDayInMonth(inLeapYear, i)) {
            month = (i - 1) as byte;
            break;
        }
    }

    return (dayWithinYear + 1 - getFirstDayInMonth(inLeapYear, month)) as byte;
}

/**
 * @see ECMA-262, 21.4.1.5
 *
 * @param time
 *
 * @returns Week day.
 */
function ecmaWeekDay(time: long): byte {
    let day = ecmaDayFromTime (time);
    let weekDay = ((day + 4) % 7) as byte;

    return weekDay >= 0 ? weekDay : (weekDay + 7) as byte;
}

/**
 * @see ECMA-262, 21.4.1.5
 *
 * @param time
 *
 * @returns Week day.
 */
function ecmaTimeInDayFromTime(time: long): long {
    let day = ecmaDayFromTime(time);
    return (time - (day * msPerDay));
}

/**
 * @see ECMA-262, 21.4.1.13
 *
 * @returns Hour component of a given time.
 */
function ecmaHourFromTime(time: long): byte {
    let timeInDay = ecmaTimeInDayFromTime(time);
    return (timeInDay / msPerHour) as byte;
}

/**
 * @see ECMA-262, 21.4.1.13
 *
 * @returns Minute component of a given time.
 */
function ecmaMinFromTime(time: long): byte {
    let timeInDay = ecmaTimeInDayFromTime(time);
    return ((timeInDay / msPerMinute) % minutesPerHour) as byte;
}

/**
 * @see ECMA-262, 21.4.1.13
 *
 * @returns Seconds component of a given time.
 */
function ecmaSecFromTime(time: long): byte {
    let timeInDay = ecmaTimeInDayFromTime(time);
    return ((timeInDay / msPerSecond) % secondsPerMinute) as byte;
}

/**
 * @see ECMA-262, 21.4.1.13
 *
 * @returns Milliseconds component of a given time.
 */
function ecmaMsFromTime(time: long): short {
    let timeInDay = ecmaTimeInDayFromTime(time);
    return (timeInDay % msPerSecond) as short;
}

/**
 * @see ECMA-262, 21.4.1.14
 *
 * @param hr
 *
 * @param min
 *
 * @param sec
 *
 * @param ms
 *
 * @returns Constructed time in milliseconds.
 */
function ecmaMakeTime(hr: long, min: long, sec: long, ms: long): long {
    return hr * msPerHour + min * msPerMinute + sec * msPerSecond + ms;
}

/**
 * @see ECMA-262, 21.4.1.14
 *
 * @param hr
 *
 * @param min
 *
 * @param sec
 *
 * @param ms
 *
 * @returns Constructed time in milliseconds.
 */
function ecmaMakeTime(hr: double, min: double, sec: double, ms: double): long {
    let hh = trunc(hr)  as long;
    let mm = trunc(min) as long;
    let ss = trunc(sec) as long;
    let mi = trunc(ms)  as long;

    return ecmaMakeTime(hh, mm, ss, mi);
}

/**
 * @see ECMA-262, 21.4.1.30
 *
 * @param year
 *
 * @returns returns the full year associated with the integer part of year
 */
function ecmaMakeFullYear(year: long): long {
    if (0 <= year && year <= 99) {
        year = year + 1900;
    }
    return year;
}

/**
 * @see ECMA-262, 21.4.1.30
 *
 * @param year
 *
 * @returns returns the full year associated with the integer part of year
 */
function ecmaMakeFullYear(year: double): long {
    let y = trunc(year)  as long;
    return ecmaMakeFullYear(y)
}

/**
 * @see ECMA-262, 21.4.1.15
 *
 * @param year
 *
 * @param month
 *
 * @param date
 *
 * @returns Elapsed number of days since Unix Epoch.
 */
function ecmaMakeDay(year: long, month: long, date: long): long {
    let ym = (year + floor(month as double / 12.0 as double)) as int;
    let mn = (month % 12) as byte;

    if (mn < 0) {
        mn += 12;
    }

    let days: long = (ecmaDayFromYear(ym) as long + getFirstDayInMonth(ecmaInLeapYear(ym), mn) as long + (date as long) - 1);

    return days;
}

/**
 * @see ECMA-262, 21.4.1.15
 *
 * @param year
 *
 * @param month
 *
 * @param day
 *
 * @returns Elapsed number of days since Unix Epoch.
 */
function ecmaMakeDay(year: double, month: double, day: double): long {
    let y = trunc(year)  as long;
    let m = trunc(month) as long;
    let d = trunc(day)   as long;

    return ecmaMakeDay(y, m, d);
}

/**
 * @see ECMA-262, 21.4.1.16
 *
 * @returns Elapsed number of milliseconds since Unix Epoch.
 *
 */
function ecmaMakeDate(day:long, time: long): long {
    return day * msPerDay + time;
}

/**
 * @see ECMA-262, 21.4.1.17
 *
 * @returns Elapsed number of milliceconds since Unix Epoch.
 *
 */
function ecmaTimeClip(time: long): long throws {
    if ((time > maxDateOffset) || (time < -maxDateOffset)) {
        throw new InvalidDate();
    }

    return time;
}

/**
 * Returns first day of a given year since epoch.
 *
 * @param year
 *
 * @returns day from year
 *
 * @see ECMA-262, 21.4.1.3, DayFromYear
 */
function ecmaDayFromYear(year: int): int {
    return (365 * (year - 1970) +
        floor((year - 1969) / 4.0) -
        floor((year - 1901) / 100.0) +
        floor((year - 1601) / 400.0)) as int;
}

/**
 * Returns first day of a given year since epoch.
 *
 * @param year
 *
 * @returns day from year
 *
 * @see ECMA-262, 21.4.1.3, DayFromYear
 */
function ecmaTimeFromYear(year: int): long {
    return msPerDay as long * ecmaDayFromYear(year) as long;
}

/**
 * The leap-year function is 1 for a time within a leap year and otherwise is 0.
 *
 * @see ECMA-262, 21.4.1.3
 *
 * @returns 1 - if the year is leap
 *          0 - otherwise
 */
function ecmaInLeapYear(year: int): byte {
    return (ecmaDaysInYear(year) - dayCountInYear) as byte;
}

/**
 * Date JS API-compatible class
 */
export class Date {
    /** Stored value of Date, milliseconds */
    private ms: long;
    private get TZOffset(): long {
        return Date.getLocalTimezoneOffset(this.ms as long);
    }

    /**
     * The `parseDateFromString()` static method parses a string representation of a date,
     * and returns the number of milliseconds since January 1, 1970, 00:00:00 UTC
     * or raises `InvalidDate` if the string is unrecognized or, in some cases,
     * contains illegal date values (e.g. 2015-02-31).
     *
     * Only the @link{https://tc39.es/ecma262/#sec-date-time-string-format|ISO 8601 format}
     * (YYYY-MM-DDTHH:mm:ss.sssZ) is explicitly specified to be supported.
     * Other formats are implementation-defined and may not work across all browsers (targets).
     * A library can help if many different formats are to be accommodated.
     *
     * @see ECMA-262, 21.4.3.2
     *
     * @returns A number representing the milliseconds elapsed since January 1, 1970, 00:00:00 UTC
     * and the date obtained by parsing the given string representation of a date.
     * If the argument doesn't represent a valid date, `InvalidDate` error is thrown.
     */
    private static parseDateFromString(dateStr: String): number {
        const dateStrLen = dateStr.length as int;
        if (dateStrLen < 4) {
            return 0;
        }
        if (dateStrLen > 35) {
            throw new InvalidDate();
        }

        let date = new Date(0);

        let timeZone: long = 0;
        const years = Date.parseIfContains(dateStr, 0, 4);
        const months = Date.parseIfContains(dateStr, 5, 7);
        const days = Date.parseIfContains(dateStr, 8, 10);
        const hours = Date.parseIfContains(dateStr, 11, 13);
        const minutes = Date.parseIfContains(dateStr, 14, 16);
        const seconds = Date.parseIfContains(dateStr, 17, 19);
        const milliseconds = Date.parseIfContains(dateStr, 20, 23);

        date.setUTCFullYear(years);
        if (months != Date.UNSUCCESSFUL_PARSING) {
            if (months > monthCountPerYear || months <= 0) {
                throw new InvalidDate();
            }
            date.setUTCMonth((months - 1) as int);
        } else {
            date.setUTCMonth(0 as int);
        }
        if (days != Date.UNSUCCESSFUL_PARSING) {
            if (days > maxDaysInMonth || days <= 0) {
                throw new InvalidDate();
            }
            date.setUTCDate((days) as byte);
        } else {
            date.setUTCDate(1 as byte);
        }
        if (hours != Date.UNSUCCESSFUL_PARSING) {
            if (hours > hoursPerDay || hours < 0) {
                throw new InvalidDate();
            }
            date.setUTCHours(hours as byte);
        } else {
            date.setUTCHours(0 as byte);
        }
        if (minutes != Date.UNSUCCESSFUL_PARSING) {
            if (minutes >= minutesPerHour || minutes < 0) {
                throw new InvalidDate();
            }
            date.setUTCMinutes(minutes as byte);
        } else {
            date.setUTCMinutes(0 as byte);
        }
        if (seconds != Date.UNSUCCESSFUL_PARSING) {
            if (seconds >= secondsPerMinute || seconds < 0) {
                throw new InvalidDate();
            }
            date.setUTCSeconds(seconds as byte);
        } else {
            date.setUTCSeconds(0 as byte);
        }
        if (milliseconds != Date.UNSUCCESSFUL_PARSING) {
            if (milliseconds >= msPerSecond || milliseconds < 0) {
                throw new InvalidDate();
            }
            date.setUTCMilliseconds(milliseconds as short);
        } else {
            date.setUTCMilliseconds(0 as short);
        }
        if (dateStrLen > 23) {
            timeZone = 0;
            let timeZoneSign = 0;
            let timeZoneSignChar = c'+';
            timeZoneSignChar = dateStr.charAt(23);
            if (timeZoneSignChar == c'-') {
                timeZoneSign = 1;
            } else if (timeZoneSignChar == c'Z') {
                timeZoneSign = 0;
            } else if (timeZoneSignChar == c'+') {
                timeZoneSign = -1;
            } else {
                throw new InvalidDate();
            }
            const timeZoneHours = Date.parseIfContains(dateStr, 24, 26);
            const timeZoneMinutes = Date.parseIfContains(dateStr, 27, 29);
            const timeZoneSeconds = Date.parseIfContains(dateStr, 30, 32);
            const timeZoneSecondsFraction = Date.parseIfContains(dateStr, 33, min(36, dateStrLen) as int);
            if (timeZoneHours != Date.UNSUCCESSFUL_PARSING) {
                if (timeZoneHours < 0 || timeZoneHours >= hoursPerDay) {
                    throw new InvalidDate();
                }
                timeZone += timeZoneHours * msPerHour;
            }
            if (timeZoneMinutes != Date.UNSUCCESSFUL_PARSING) {
                if (timeZoneMinutes < 0 || timeZoneMinutes >= minutesPerHour) {
                    throw new InvalidDate();
                }
                timeZone += timeZoneMinutes * msPerMinute;
            }
            if (timeZoneSeconds != Date.UNSUCCESSFUL_PARSING) {
                if (timeZoneSeconds < 0 || timeZoneSeconds >= secondsPerMinute) {
                    throw new InvalidDate();
                }
                timeZone += timeZoneSeconds * msPerSecond;
            }
            if (timeZoneSecondsFraction != Date.UNSUCCESSFUL_PARSING) {
                let timeZoneMillis = timeZoneSecondsFraction;
                if (dateStrLen == 32) {
                    timeZoneMillis *= 100;
                } else if (dateStrLen == 33) {
                    timeZoneMillis *= 10;
                }
                if (timeZoneSecondsFraction < 0) {
                    throw new InvalidDate();
                }
                timeZone += timeZoneSecondsFraction;
            }

            timeZone *= timeZoneSign;
        }
        if (dateStrLen == 23) {
            timeZone = Date.getLocalTimezoneOffset(date.valueOf() as long) *  msPerMinute;
        }
        return date.valueOf() + timeZone;
    }
    /**
     * Default constructor.
     *
     * @see ECMA-262, 21.4.2.1
     *
     * @description Initializes Date instance with current time.
     */
    constructor() {
        this.ms = Date.now() as long;
    }

    /**
     * `Date` constructor.
     *
     * @param value
     *
     * @see ECMA-262, 21.4.2.1
     *
     * @description Initializes `Date` instance with `number` or `string` or another `Date` instance.
     * NOTE: Dates before `1921-01-01T00:00:00 GMT` can be represented as UTC milliseconds in different ways then TS do.
     */

    constructor(value: number | string | Date) {

        if (value instanceof Date) {
            this.ms = trunc((value as Date).valueOf() as number) as long
        }
        else if (typeof value == "string") {
            this.ms = (Date.parseDateFromString(value as string) as long);
        }
        else {
            if (isNaN(value as number)) {
                this.ms = (maxDateOffset + 1) as long;
            } else {
                this.ms = trunc(value as number) as long;
            }
        }
    }

    /**
     * `Date` constructor.
     *
     * @param year
     *
     * @param month
     *
     * @param day
     *
     * @param hours
     *
     * @param minutes
     *
     * @param seconds
     *
     * @param ms
     *
     * @see ECMA-262, 21.4.2.1
     *
     * @description Initialize `Date` instance with year, month, day, hours, minutes, seconds and milliseconds given.
     * NOTE: Dates before `1921-01-01T00:00:00 GMT` can be represented as UTC milliseconds in different ways then TS do.
     */
    constructor(year: number, monthIndex: number, day?: number, hours?: number, minutes?: number, seconds?: number, ms?: number) {
        this.ms = ecmaMakeDate(
            ecmaMakeDay(
                ecmaMakeFullYear(year),
                monthIndex,
                day != undefined ? day as number: 1
            ),
            ecmaMakeTime(
                hours != undefined ? hours as number : 0,
                minutes != undefined ? minutes as number : 0,
                seconds != undefined ? seconds as number : 0,
                ms != undefined ? ms as number : 0
            )
        );
        this.ms += this.TZOffset * msPerMinute
    }

    /**
     * Creates a new instance of a Date as a string
     *
     * @returns A new Date instance as a string
     */
    static $_invoke(): string {
        return new Date().toString();
    }

    /**
     * The `isDateValid()` method checks if given date `ms` is maximum of ±100,000,000
     * (one hundred million) days relative to January 1, 1970 UTC
     * (that is, April 20, 271821 BCE ~ September 13, 275760 CE) can be represented
     * by the standard `Date` object (equivalent to ±8,640,000,000,000,000 milliseconds).
     */
    private isDateValid(ms: long): boolean {
        return ms >= -maxDateOffset && ms <= maxDateOffset;
    }

    /**
     * The `isDateValid()` method checks if constructed date is maximum of ±100,000,000
     * (one hundred million) days relative to January 1, 1970 UTC
     * (that is, April 20, 271821 BCE ~ September 13, 275760 CE) can be represented
     * by the standard Date object (equivalent to ±8,640,000,000,000,000 milliseconds).
     *
     */
    public isDateValid(): boolean {
        return this.isDateValid(this.ms);
    }

    /**
     * The `valueOf()` method returns the primitive value of a `Date` object.
     *
     * @see ECMA-262, 21.4.4.44
     *
     * @returns The number of milliseconds between 1 January 1970 00:00:00 UTC and the given date.
     *
     * @throws InvalidDate - Throws if Date object is invalid (@link{isDateValid} is `false`).
     */
    public valueOf(): number {
        if (!this.isDateValid()) {
            return NaN;
        }
        return this.ms as number;
    }

    /**
     * The `now()` static method returns the number of milliseconds elapsed since the epoch,
     * which is defined as the midnight at the beginning of January 1, 1970, UTC.
     *
     * @see ECMA-262, 21.4.3.1
     *
     * @returns A number representing the number of milliseconds elapsed since the epoch,
     * which is defined as the midnight at the beginning of January 1, 1970, UTC.
     */
    public static native now(): number;

    /**
     * Gets local time offset.
     *
     * @returns local time offset.
     */
    public static native getLocalTimezoneOffset(ms: long): long;

    /**
     * Gets time zone name.
     *
     * @returns time zone name.
     */
    public static native getTimezoneName(ms: long): String;

    /**
     * Gets locale string representation according to format.
     *
     * @param format
     *
     * @param locale
     *
     * @param ms
     *
     * @param isUTC
     *
     * @returns locale string in the specified format.
     */
    public static native getLocaleString(format: String, locale: String, ms: long, isUTC: boolean): String;

    /**
     * Gets a string with a language-sensitive representation of the time portion of the date.
     *
     * @returns a language-sensitive representation of the time portion of the date.
     */
    public toLocaleTimeString(): string {
        return Date.getLocaleString("%EX", "", this.ms, false);
    }

    /**
     * Gets a string with a language-sensitive representation of the time portion of the date with respect to locale.
     *
     * @param locale
     *
     * @returns a language-sensitive representation of the time portion of the date with respect to locale.
     */
    public toLocaleTimeString(locale: string): string {
        return Date.getLocaleString("%EX", locale, this.ms, false);
    }

    /**
     * Gets a string with a language-sensitive representation of this date.
     *
     * @returns a language-sensitive representation of this date.
     */
    public toLocaleString(): string {
        return Date.getLocaleString("%Ex %EX", "", this.ms, false);
    }

    /**
     * Gets a string with a language-sensitive representation of this date with respect to locale.
     *
     * @param locale
     *
     * @returns a language-sensitive representation of this date.
     */
    public toLocaleString(locale: string): string {
        return Date.getLocaleString("%Ex %EX", locale, this.ms, false);
    }

    /**
     * Gets a string with a language-sensitive representation
     * of the date portion of the specified date in the user agent's timezone.
     *
     * @returns a string with a language-sensitive representation
     * of the date portion of the specified date in the user agent's timezone.
     */
    public toLocaleDateString(): string {
        return Date.getLocaleString("%Ex", "", this.ms, false);
    }

    /**
     * Returns a string with a language-sensitive representation
     * of the date portion of the specified date in the user agent's timezone.
     *
     * @returns a string with a language-sensitive representation
     * of the date portion of the specified date in the user agent's timezone.
     */
    public toLocaleDateString(locale: string): string {
        return Date.getLocaleString("%Ex", locale, this.ms, false);
    }

    /**
     * The `getDate()` method returns the day of the month for the specified date according to local time.
     *
     * @see ECMA-262, 21.4.4.2
     *
     * @returns An integer number, between 1 and 31, representing the day of the month for the given date according to local time.
     */
    public getDate(): number {
        let localTime = this.ms - this.TZOffset * 60 * msPerSecond;
        let day: int = ecmaDayFromTime(localTime) - ecmaDayFromYear(ecmaYearFromTime(localTime));
        let inLeapYear = ecmaInLeapYear(this.getFullYear() as int);
        let firstDay: int = getFirstDayInMonth(inLeapYear, ecmaMonthFromTime(localTime));
        return (day - firstDay + 1) as number;
    }

    /**
     * Changes the day of the month of a given Date instance, based on local time.
     *
     * @param value new day.
     */
    public setDate(value: byte): void {
        let day = this.getDate();
        this.ms -= day * msPerDay;
        this.ms += value * msPerDay;
    }

    /**
     * Changes the day of the month of a given Date instance, based on local time.
     *
     * @param value new day.
     */
    public setDate(value: number): number {
        this.setDate(value as byte);
        return this.ms as number;
    }

    /**
     * Alias to @link{setDate} and left for compatibility with ECMA-262.
     *
     * @param value new day.
     */
    public setDay(value: byte): void {
        this.setDate(value);
    }

    /**
     * Returns the day of the month (from 1 to 31) in the specified date according to universal time.
     *
     * @returns An integer number, between 1 and 31, representing the day of the month for the given date according to local time.
     */
    public getUTCDate(): number {
        let localTime = this.ms;
        let day: int = ecmaDayFromTime(localTime) - ecmaDayFromYear(ecmaYearFromTime(localTime));
        let inLeapYear = ecmaInLeapYear(this.getFullYear() as int);
        let firstDay: int = getFirstDayInMonth(inLeapYear, ecmaMonthFromTime(localTime));
        return (day - firstDay + 1) as number;
    }

    /**
     * Changes the day of the month of a given Date instance, based on UTC time.
     *
     * @param value new day.
     */
    public setUTCDate(value: byte): void {
        let day = this.getUTCDate() as byte;
        this.ms -= day * msPerDay;
        this.ms += value * msPerDay;
    }

    /**
     * Changes the day of the month of a given Date instance, based on UTC time.
     *
     * @param value new day.
     */
    public setUTCDate(value: number): number {
        this.setUTCDate(value as byte);
        return this.ms as number;
    }

    /**
     * Changes the day of the month of a given Date instance, based on UTC time.
     *
     * @param value new day.
     */
    public setUTCDay(value: byte): void {
        this.setUTCDate(value);
    }

    /**
     * Changes the day of the month of a given Date instance, based on UTC time.
     *
     * @param value new day.
     */
    public setUTCDay(value: number): number {
        this.setUTCDate(value);
        return this.ms as number;
    }

    /**
     * Returns the day of the week for the specified date according to local time,
     * where 0 represents Sunday. For the day of the month, see @link{getDayOfMonth}.
     *
     * @see ECMA-262, 21.4.4.3
     *
     * @returns An integer number, between 0 and 6, corresponding to the day of the week
     * for the given date, according to local time: 0 for Sunday, 1 for Monday, 2 for Tuesday, and so on.
     */
    public getDay(): number {
        return ecmaWeekDay(this.ms - this.TZOffset * 60 * msPerSecond) as number;
    }

    /**
     * Returns the day of the week in the specified date according to universal time, where 0 represents Sunday.
     *
     * @returns An integer number, between 0 and 6, corresponding to the day of the week
     * for the given date, according to local time: 0 for Sunday, 1 for Monday, 2 for Tuesday, and so on.
     */
    public getUTCDay(): number {
        return ecmaWeekDay(this.ms) as number;
    }

    /**
     * Returns the year of the specified date according to local time.
     *
     * @see ECMA-262, 21.4.4.4
     * deprecated
     * @note This function is an alias to @link{getFullYear} and left for compatibility with ECMA-262.
     *
     * @returns year
     */
    public getYear(): int {
        return this.getFullYear() as int;
    }

    /**
     * Returns the year of the specified date according to local time.
     *
     * @returns A year of the specified date according to local time.
     *
     * @description The value returned by `getUTCFullYear()` is an absolute number.
     * For dates between the years 1000 and 9999, `getUTCFullYear()` returns a four-digit number,
     * for example, 1995. Use this function to make sure a year is compliant with years after 2000.
     *
     * @returns year
     */
    public getUTCFullYear(): number {
        return ecmaYearFromTime(this.ms) as number;
    }

    /**
     * Returns the year of the specified date according to local time.
     *
     * @see ECMA-262, 21.4.4.4
     *
     * @returns A number corresponding to the year of the given date, according to local time.
     *
     * @description The value returned by `getFullYear()` is an absolute number.
     * For dates between the years 1000 and 9999, `getFullYear()` returns a four-digit number,
     * for example, 1995. Use this function to make sure a year is compliant with years after 2000.
     *
     * @example
     * ```ets
     * const today = new Date();
     * const year = today.getYearFull();
     * ```
     */
    public getFullYear(): number {
        let localTime = this.ms - this.TZOffset * 60 * msPerSecond;
        return ecmaYearFromTime(localTime) as number;
    }

    /**
     * Sets the full year for a specified date according to universal time.
     *
     * @param value new year
     */
    public setUTCFullYear(value: number, month?: number, date?: number): number {
        this.setUTCFullYear(value as int);
        if (month !== undefined) {
            this.setUTCMonth(month.intValue());
        }
        if (date !== undefined) {
            this.setUTCDate(date.byteValue());
        }
        return this.ms as number;
    }

    /**
     * Sets the full year for a specified date according to universal time.
     *
     * @param value new year
     */
    public setUTCFullYear(value: int): void {
        let year = ecmaYearFromTime(this.ms);
        let month = ecmaMonthFromTime(this.ms)
        this.ms -= ecmaMakeDay(year, month, 1) * msPerDay as long
        this.ms += ecmaMakeDay(value, month, 1) * msPerDay as long
    }

    /**
     * This function is an alias to @link{setFullYear} and left for compatibility with ECMA-262.
     *
     * @param value new year
     */
    public setYear(value: number): void {
        this.setYear(value as int)
    }

    /**
     * This function is an alias to @link{setFullYear} and left for compatibility with ECMA-262.
     *
     * @param value new year
     */
    public setYear(value: int): void {
        this.setFullYear(value);
    }

    /**
     * Sets the full year for a specified date according to local time.
     *
     * @param value new year
     */
    public setFullYear(value: number, month?: Number, date?: Number): number {
        this.setFullYear(value as int);
        if (month !== undefined) {
            this.setMonth(month.intValue());
        }
        if (date !== undefined) {
            this.setDate(date.byteValue());
        }
        return this.ms as number;
    }

    /**
     * Sets the full year for a specified date according to local time.
     *
     * @param value new year
     */
    public setFullYear(value: int): void {
        let local_time = this.ms - this.TZOffset * 60 * msPerSecond
        let year = ecmaYearFromTime(local_time)
        let month = ecmaMonthFromTime(local_time)
        this.ms -= ecmaMakeDay(year, month, 1) * msPerDay as long
        this.ms += ecmaMakeDay(value, month, 1) * msPerDay as long
    }

    /**
     * Returns the hour for the specified date, according to local time.
     *
     * @see ECMA-262, 21.4.4.5
     *
     * @returns An integer number, between 0 and 23, representing the hour for the given date according to local time.
     *
     * @example
     * ```ets
     * const today = new Date();
     * const hours = today.getHour();
     * ```
     */
    public getHours(): number {
        let localTime = this.ms - this.TZOffset * 60 * msPerSecond;
        return ecmaHourFromTime(localTime) as number;
    }

    /**
     * Returns the hours in the specified date according to universal time.
     *
     * @returns An integer number, between 0 and 23, representing the hour for the given date according to UTC time.
     */
    public getUTCHours(): number {
        return ecmaHourFromTime(this.ms) as number;
    }

    /**
     * Sets the hours for a specified date according to local time.
     *
     * @param value new hours
     */
    public setHours(value: byte): void {
        assertTrue(value >= 0 && value <= 23, "Invalid value " + value + " for \"value\", has to be in range 0..23");
        let hours = ecmaHourFromTime(this.ms - this.TZOffset * 60 * msPerSecond);
        this.ms -= msPerHour * hours;
        this.ms += msPerHour * value;
    }

    /**
     * Sets the hours for a specified date according to local time.
     *
     * @param value new hours
     */
    public setHours(value: number, min?: number, sec?: number, ms?: number): number {
        this.setHours(value as byte);
        if (min !== undefined) {
            this.setMinutes(min.byteValue());
        }
        if (sec !== undefined) {
            this.setSeconds(sec.byteValue());
        }
        if (ms !== undefined) {
            this.setMilliseconds(ms.shortValue());
        }
        return this.ms as number;
    }

    /**
     * Sets the hour for a specified date according to universal time.
     *
     * @param value new hours
     */
    public setUTCHours(value: byte): void {
        assertTrue(value >= 0 && value <= 23, "Invalid value " + value + " for \"value\", has to be in range 0..23");
        let hours = ecmaHourFromTime(this.ms);
        this.ms -= msPerHour * hours;
        this.ms += msPerHour * value;
    }

    /**
     * Sets the hour for a specified date according to universal time.
     *
     * @param value new hours
     */
    public setUTCHours(value: number, min?: number, sec?: number, ms?: number): number {
        this.setUTCHours(value as byte);
        if (min !== undefined) {
            this.setUTCMinutes(min.byteValue());
        }
        if (sec !== undefined) {
            this.setUTCSeconds(sec.byteValue());
        }
        if (ms !== undefined) {
            this.setUTCMilliseconds(ms.shortValue());
        }
        return this.ms as number;
    }

    /**
     * Returns the milliseconds in the specified date according to local time.
     *
     * @see ECMA-262, 21.4.4.6
     *
     * @returns A number between 0 and 999 representing the milliseconds for the given date according to local time.
     *
     * @example
     * ```ets
     * const today = new Date();
     * const milliseconds = today.getMilliseconds();
     * ```
     */
    public getMilliseconds(): number {
        let localTime = this.ms - this.TZOffset * 60 * msPerSecond;
        return ecmaMsFromTime(localTime) as number;
    }

    /**
     * Returns the number of milliseconds elapsed since the epoch,
     * which is defined as the midnight at the beginning of January 1, 1970, UTC.
     *
     * @param d to be converted to milliseconds.
     *
     * @see ECMA-262, 21.4.3.1
     *
     * @returns A number representing the number of milliseconds elapsed since the epoch,
     * which is defined as the midnight at the beginning of January 1, 1970, UTC.
     */
    public static UTC(d: Date): long {
        return d.valueOf() as long;
    }

    /**
     * Returns the number of milliseconds elapsed since the epoch,
     * which is defined as the midnight at the beginning of January 1, 1970, UTC.
     *
     * @param year to be converted to milliseconds.
     *
     * @returns A number representing the number of milliseconds elapsed since the epoch,
     * which is defined as the midnight at the beginning of January 1, 1970, UTC.
     */
    public static UTC(year: number): number {
        return ecmaMakeDate(ecmaMakeDay(ecmaMakeFullYear(year), 0.0, 1.0), ecmaMakeTime(0.0, 0.0, 0.0, 0.0)) as number
    }

    /**
     * Returns the number of milliseconds elapsed since the epoch,
     * which is defined as the midnight at the beginning of January 1, 1970, UTC.
     *
     * @param year to be converted to milliseconds.
     *
     * @param month to be converted to milliseconds.
     *
     * @param day to be converted to milliseconds.
     *
     * @param hours to be converted to milliseconds.
     *
     * @param minutes to be converted to milliseconds.
     *
     * @param seconds to be converted to milliseconds.
     *
     * @param ms to be converted to milliseconds.
     *
     * @returns A number representing the number of milliseconds elapsed since the epoch,
     * which is defined as the midnight at the beginning of January 1, 1970, UTC.
     */

    public static UTC(year: number, month: number, day?: number, hours?: number, minutes?: number, seconds?: number, ms?: number): number {
        return ecmaMakeDate(
            ecmaMakeDay(
                ecmaMakeFullYear(year),
                month,
                day != undefined ? day as number: 1.0
            ),
            ecmaMakeTime(
                hours != undefined ? hours as number : 0,
                minutes != undefined ? minutes as number : 0,
                seconds != undefined ? seconds as number : 0,
                ms != undefined ? ms as number : 0
            )
        ) as number;
    }

    /**
     * Returns the milliseconds portion of the time object's value according to universal time.
     *
     * @returns the milliseconds portion of the time object's value according to universal time.
     */
    public getUTCMilliseconds(): number {
        let localTime = this.ms;
        return ecmaMsFromTime(localTime) as number;
    }

    /**
     * Sets the milliseconds for a specified date according to local time.
     *
     * @param value new ms
     */
    public setMilliseconds(value: short): void {
        let ms = ecmaMsFromTime(this.ms - this.TZOffset * 60 * msPerSecond);
        this.ms -= ms;
        this.ms += value;
    }

    /**
     * Sets the milliseconds for a specified date according to local time.
     *
     * @param value new ms
     */
    public setMilliseconds(value: number): number {
        this.setMilliseconds(value as short);
        return this.ms as number;
    }

    /**
     * Sets the milliseconds for a specified date according to universal time.
     *
     * @param value new ms
     */
    public setUTCMilliseconds(value: short): void {
        let ms = ecmaMsFromTime(this.ms);
        this.ms -= ms;
        this.ms += value;
    }

    /**
     * Sets the milliseconds for a specified date according to universal time.
     *
     * @param value new ms
     */
    public setUTCMilliseconds(value: number): number {
        this.setUTCMilliseconds(value as short);
        return this.ms as number;
    }

    /**
     * Returns the seconds in the specified date according to local time.
     *
     * @see ECMA-262, 21.4.4.9
     *
     * @returns An integer number, between 0 and 59, representing the seconds in the given date according to local time.
     *
     * @example
     * ```ets
     * const today = new Date();
     * const seconds = today.getSeconds();
     * ```
     */
    public getSeconds(): number {
        let localTime = this.ms - this.TZOffset * 60 * msPerSecond;
        return ecmaSecFromTime(localTime) as number;
    }

    /**
     * Returns the seconds in the specified date according to universal time.
     *
     * @returns the seconds in the specified date according to universal time.
     */
    public getUTCSeconds(): number {
        return ecmaSecFromTime(this.ms) as number;
    }

    /**
     * Sets the seconds for a specified date according to local time.
     *
     * @param value new seconds
     */
    public setSeconds(value: number, ms?: number): number {
        this.setSeconds(value as byte);
        if (ms !== undefined) {
            this.setMilliseconds(ms.shortValue());
        }
        return this.ms as number;
    }

    /**
     * Sets the seconds for a specified date according to local time.
     *
     * @param value new seconds
     */
    public setSeconds(value: byte): void {
        assertTrue(value >= 0 && value < 60, "Invalid value " + value + " for \"value\", has to be in range 0..60");
        let sec = ecmaSecFromTime(this.ms - this.TZOffset * 60 * msPerSecond);
        this.ms -= sec * msPerSecond;
        this.ms += value * msPerSecond;
    }

    /**
     * Sets the seconds for a specified date according to universal time.
     *
     * @param value new seconds
     */
    public setUTCSeconds(value: byte): void {
        assertTrue(value >= 0 && value < 60, "Invalid value " + value + " for \"value\", has to be in range 0..60");
        let sec = ecmaSecFromTime(this.ms);
        this.ms -= sec * msPerSecond;
        this.ms += value * msPerSecond;
    }

    /**
     * Sets the seconds for a specified date according to universal time.
     *
     * @param value new seconds
     */
    public setUTCSeconds(value: number, ms?: number): number {
        this.setUTCSeconds(value as byte);
        if (ms !== undefined) {
            this.setUTCMilliseconds(ms.shortValue());
        }
        return this.ms as number;
    }

    /**
     * Returns the minutes in the specified date according to local time.
     *
     * @see ECMA-262, 21.4.4.7
     *
     * @returns An integer number, between 0 and 59, representing the minutes in the given date according to local time.
     *
     * @example
     * ```ets
     * const today = new Date();
     * const minutes = today.getMinutes();
     * ```
     */
    public getMinutes(): number {
        let localTime = this.ms - this.TZOffset * 60 * msPerSecond;
        return ecmaMinFromTime(localTime) as number;
    }

    /**
     * Sets the minutes for a specified date according to universal time.
     *
     * @param value new minutes
     */
    public setUTCMinutes(value: byte): void {
        assertTrue(value >= 0 && value < 60, "Invalid value " + value + " for \"value\", has to be in range 0..60");
        let min = ecmaMinFromTime(this.ms);
        this.ms -= min * msPerMinute;
        this.ms += value * msPerMinute;
    }

    /**
     * Sets the minutes for a specified date according to universal time.
     *
     * @param value new minutes
     */
    public setUTCMinutes(value: number, sec?: Number, ms?: Number): number {
        this.setUTCMinutes(value as byte);
        if (sec !== undefined) {
            this.setUTCSeconds(sec.byteValue());
        }
        if (ms !== undefined) {
            this.setUTCMilliseconds(ms.shortValue());
        }
        return this.ms as number;
    }

    /**
     * Returns the minutes in the specified date according to universal time.
     *
     * @returns the minutes in the specified date according to universal time.
     */
    public getUTCMinutes(): number {
        return ecmaMinFromTime(this.ms) as number;
    }

    /**
     * Sets the minutes for a specified date according to local time.
     *
     * @param value new minutes
     */
    public setMinutes(value: byte): void {
        assertTrue(value >= 0 && value < 60, "Invalid value " + value + " for \"value\", has to be in range 0..60");
        let min = ecmaMinFromTime(this.ms - this.TZOffset * 60 * msPerSecond);
        this.ms -= min * msPerMinute;
        this.ms += value * msPerMinute;
    }

    /**
     * Sets the minutes for a specified date according to local time.
     *
     * @param value new minutes
     */
    public setMinutes(value: number, sec?: number, ms?: number): number {
        this.setMinutes(value as byte);
        if (sec !== undefined) {
            this.setSeconds(sec.byteValue());
        }
        if (ms !== undefined) {
            this.setMilliseconds(ms.shortValue());
        }
        return this.ms as number;
    }

    /**
     * Returns the month in the specified date according to local time,
     * as a zero-based value (where zero indicates the first month of the year).
     *
     * @see ECMA-262, 21.4.4.8
     *
     * @returns  An integer number, between 0 and 11, representing the month in the given date according to local time.
     * 0 corresponds to January, 1 to February, and so on.
     *
     * @example
     * ```ets
     * const today = new Date();
     * const month = today.getMonth();
     * ```
     */
    public getMonth(): number {
        let localTime = this.ms - this.TZOffset * 60 * msPerSecond;
        return ecmaMonthFromTime(localTime) as number;
    }

    /**
     * Returns the month of the specified date according to universal time, as a zero-based value (where zero indicates the first month of the year).
     *
     * @returns  An integer number, between 0 and 11, representing the month in the given date according to UTC time.
     * 0 corresponds to January, 1 to February, and so on.
     */
    public getUTCMonth(): number {
        return ecmaMonthFromTime(this.ms) as number;
    }

    /**
     * Sets the month for a specified date according to the currently set year.
     *
     * @param month new month
     */
    public setMonth(month: number, date?: number): number {
        this.setMonth(month as int);
        if (date !== undefined) {
            this.setDate(date.byteValue());
        }
        return this.ms as number;
    }

    /**
     * Sets the month for a specified date according to the currently set year.
     *
     * @param month new month
     */
    public setMonth(month: int): void {
        let inLeapYear = ecmaInLeapYear(this.getFullYear() as int);
        let daysNow: int = getFirstDayInMonth(inLeapYear, ecmaMonthFromTime(this.ms - this.TZOffset * 60 * msPerSecond));
        let daysNew: int = getFirstDayInMonth(inLeapYear, month);
        this.ms -= daysNow * msPerDay;
        this.ms += daysNew * msPerDay;
    }

    /**
     * Sets the month for a specified date according to universal time.
     *
     * @param month new month
     */
    public setUTCMonth(month: number, date?: number): number {
        this.setUTCMonth(month as int);
        if (date !== undefined) {
            this.setUTCDate(date.byteValue());
        }
        return this.ms as number;
    }

    /**
     * Sets the month for a specified date according to universal time.
     *
     * @param month new month
     */
    public setUTCMonth(month: int): void {
        let inLeapYear = ecmaInLeapYear(this.getFullYear() as int);
        let daysNow: int = getFirstDayInMonth(inLeapYear, ecmaMonthFromTime(this.ms));
        let daysNew: int = getFirstDayInMonth(inLeapYear, month);
        this.ms -= daysNow * msPerDay;
        this.ms += daysNew * msPerDay;
    }

    /**
     * Returns the number of milliseconds since the epoch,
     * which is defined as the midnight at the beginning of January 1, 1970, UTC.
     *
     * @see ECMA-262, 21.4.4.10
     *
     * @returns A number representing the milliseconds elapsed between 1 January 1970 00:00:00 UTC and the given date.
     */
    public getTime(): number {
        return this.valueOf();
    }

    /**
     * Sets the number of milliseconds since the epoch,
     * which is defined as the midnight at the beginning of January 1, 1970, UTC.
     *
     * @param value new ms
     *
     * @see ECMA-262, 21.4.4.10
     *
     * @returns A number representing the milliseconds elapsed between 1 January 1970 00:00:00 UTC and the given date.
     */
    public setTime(value: long): void {
        this.ms = value;
    }

    /**
     * Sets the number of milliseconds since the epoch,
     * which is defined as the midnight at the beginning of January 1, 1970, UTC.
     *
     * @param value new ms
     *
     * @see ECMA-262, 21.4.4.10
     *
     * @returns A number representing the milliseconds elapsed between 1 January 1970 00:00:00 UTC and the given date.
     */
    public setTime(value: number): number {
        this.setTime(value as long);
        return this.ms as number;
    }

    /**
     * Parses a string representation of a date,
     * and returns the number of milliseconds since January 1, 1970, 00:00:00 UTC
     * or raises `InvalidDate` if the string is unrecognized or, in some cases,
     * contains illegal date values (e.g. 2015-02-31).
     *
     * Only the @link{https://tc39.es/ecma262/#sec-date-time-string-format|ISO 8601 format}
     * (YYYY-MM-DDTHH:mm:ss.sssZ) is explicitly specified to be supported.
     * Other formats are implementation-defined and may not work across all browsers (targets).
     * A library can help if many different formats are to be accommodated.
     *
     * @param dateStr to be parsed
     *
     * @see ECMA-262, 21.4.3.2
     *
     * @returns A number representing the milliseconds elapsed since January 1, 1970, 00:00:00 UTC
     * and the date obtained by parsing the given string representation of a date.
     * If the argument doesn't represent a valid date, `InvalidDate` exception is thrown.
     */
    public static parse(dateStr: string): number {
        return Date.parseDateFromString(dateStr) as number;
    }

    /**
     * Returns the difference, in minutes, between a date as
     * evaluated in the UTC time zone, and the same date as evaluated in the local time zone.
     *
     * @returns the difference, in minutes, between a date as
     * evaluated in the UTC time zone, and the same date as evaluated in the local time zone.
     */
    public getTimezoneOffset(): number {
        return this.TZOffset as number;
    }

    /**
     * Sets the difference, in minutes, between a date as
     * evaluated in the UTC time zone, and the same date as evaluated in the local time zone.
     *
     * @param value new timezone offset
     */
    public setTimezoneOffset(value: number): number {
        return this.setTimezoneOffset(value as int) as number
    }

    /**
     * Returns a string in simplified extended ISO format (ISO 8601),
     * which is always 24 or 27 characters long (YYYY-MM-DDTHH:mm:ss.sssZ or
     * ±YYYYYY-MM-DDTHH:mm:ss.sssZ, respectively). The timezone is always zero UTC offset,
     * as denoted by the suffix Z.
     *
     * @see ECMA-262, 21.4.4.36
     *
     * @returns A string representing the given date in the ISO 8601 format according to universal time.
     * It's the same format as the one required to be recognized by @link{parse()}.
     *
     * @example
     * ```ets
     * const today = new Date();
     * console.println(today.toISOString()); // Returns 2023-02-05T14:48:00.000Z
     * ```
     */
    public toISOString(): string {
        let sb = new StringBuilder();
        let y = this.getUTCFullYear();
        if (y < 0) {
            sb.append("-");
        }
        if (abs(y) < 1000) {
            sb.append("0");
        }
        if (abs(y) < 100) {
            sb.append("0");
        }
        if (abs(y) < 10) {
            sb.append("0");
        }
        sb.append(abs(y) as long);
        sb.append("-");
        let month = this.getUTCMonth() + 1;
        if(month < 10){
            sb.append("0");
        }
        sb.append(month);
        sb.append("-");
        let day = this.getUTCDate();
        if(day < 10){
            sb.append("0");
        }
        sb.append(day);
        sb.append("T");
        let h = this.getUTCHours();
        if(h < 10) {
            sb.append("0");
        }
        sb.append(h);
        sb.append(":");
        let m = this.getUTCMinutes();
        if(m < 10) {
            sb.append("0");
        }
        sb.append(m);
        sb.append(":");
        let s = this.getUTCSeconds();
        if(s < 10) {
            sb.append("0");
        }
        sb.append(s);
        sb.append(".");
        let ms = this.getUTCMilliseconds();
        if (ms < 10) {
           sb.append("00")
        }
        else if (ms < 100) {
            sb.append("0")
        }
        // sb.append(ms / 100);
        // sb.append((ms / 10) % 10);
        // sb.append(ms % 10);
        sb.append(trunc(ms))
        sb.append("Z");
        return sb.toString();
    }

    /**
     * Returns a string representation of the Date object.
     *
     * @returns JSON representation of the current instance
     */
    public toJSON(): string {
        return this.toISOString();
    }

    /**
     * Returns the time portion of a `Date` object interpreted in the local timezone in English.
     *
     * @see ECMA-262, 21.4.4.42
     *
     * @returns A string representing the time portion of the given date in human-readable form in English.
     *
     * @example
     * ```
     * let d = new Date(1979.0, 9.0, 27.0, 13.0, 12.8, 57.0, 444.1);
     * console.println(d.toTimeString()); // 13:12:57 GMT
     * ```
     */
    public toTimeString(): string {
        return this.timeString() + " " + this.timeZoneString();
    }

    /**
     * Returns string representation
     */
    private timeString(): String {
        if (!this.isDateValid()) {
            throw new Error("Invalid Date");
        }

        let sb = new StringBuilder();
        let h = this.getHours();
        if (h < 10) {
            sb.append("0");
        }
        sb.append(h);
        sb.append(":");
        let m = this.getMinutes();
        if (m < 10) {
            sb.append("0");
        }
        sb.append(m);
        sb.append(":");
        let s = this.getSeconds();
        if (s < 10) {
            sb.append("0");
        }
        sb.append(s);
        return sb.toString();
    }

    /**
     * Returns the date portion of a `Date` object interpreted in the local timezone in English.
     *
     * @see ECMA-262, 21.4.4.35
     *
     * @returns A string representing the date portion of the given Date object in human-readable form in English.
     *
     * @example
     * ```
     * let d = new Date(1979.0, 9.0, 27.0, 13.0, 12.8, 57.0, 444.1);
     * console.println(d.toDateString()); // Sat Oct 27 1979
     * ```
     */
    public toDateString(): string {
        return this.dateString();
    }

    /**
     * Returns a string representation
     */
    private dateString(): String {
        if (!this.isDateValid()) {
            throw new Error("Invalid Date");
        }

        let sb = new StringBuilder();
        sb.append(dayNames[this.getDay() as int]);
        sb.append(" ");
        sb.append(monthNames[this.getMonth() as int]);
        sb.append(" ");
        let d = this.getDate();
        if (d < 10) {
            sb.append("0");
        }
        sb.append(d);
        sb.append(" ");
        let y = this.getFullYear();
        if (y < 0) {
            sb.append("-");
        }
        if (abs(y) < 1000) {
            sb.append("0");
        }
        if (abs(y) < 100) {
            sb.append("0");
        }
        if (abs(y) < 10) {
            sb.append("0");
        }
        sb.append(abs(y) as long);
        return sb.toString();
    }

    /**
     * TODO Until TZ support implemented, the time is calculated in UTC+0 and TZ string is hardcoded
     */
    private timeZoneString(): String {
        let sb = new StringBuilder();
        sb.append("GMT");
        let offset = -this.TZOffset;
        let hours = offset / 60;
        if(hours < 0) {
            sb.append("-");
        } else {
            sb.append("+")
        }
        if (abs(hours) < 10) {
            sb.append("0");
        }
        sb.append(abs(hours));
        let minutes = abs(offset % 60);
        if (minutes < 10) {
            sb.append("0");
        }
        sb.append(minutes);

        return sb.toString();
    }
    /**
     * Returns a string representing the specified `Date` object interpreted in the local timezone.
     *
     * @see ECMA-262, 21.4.4.41
     *
     * @returns A string representing the given date.
     *
     * @example
     * ```
     * let d = new Date(1979.0, 9.0, 27.0, 13.0, 12.8, 57.0, 444.1);
     * console.println(d.toString()); // Sat Oct 27 1979 13:12:57 GMT
     * ```
     */
    public override toString(): string {
        let res: String = "Invalid date";
        try {
            res = this.toDateString() + " " + this.toTimeString();
        }
        catch (e) {}
        return res;
    }

    /**
     * Returns a string representing the specified `Date` object interpreted in UTC.
     *
     * @see ECMA-262, 21.4.4.41
     *
     * @returns A string representing the given date.
     *
     * @example
     * ```
     * let d = new Date(1979.0, 9.0, 27.0, 13.0, 12.8, 57.0, 444.1);
     * console.println(d.toUTCString()); // Sat Oct 27 1979 13:12:57 GMT
     * ```
     */
    public toUTCString(): string {
        if (!this.isDateValid()) {
            throw new Error("Invalid Date");
        }
        let s: String = this.toUTCDateString() + " " + this.toUTCTimeString();
        return s;
    }

    private toUTCDateString(): string {
        let sb = new StringBuilder();
        sb.append(dayNames[this.getUTCDay() as int]);
        sb.append(", ");
        let d = this.getUTCDate();
        if (d < 10) {
            sb.append("0");
        }
        sb.append(d);
        sb.append(" ");
        sb.append(monthNames[this.getUTCMonth() as int]);
        sb.append(" ");
        let y = this.getUTCFullYear();
        if (y < 0) {
            sb.append("-");
        }
        y = abs(y);
        if (y < 1000) {
            sb.append("0");
        }
        if (y < 100) {
            sb.append("0");
        }
        if (y < 10) {
            sb.append("0");
        }
        sb.append(y as long);
        return sb.toString();
    }

    private toUTCTimeString(): string {
        let sb = new StringBuilder();
        let h = this.getUTCHours();
        if (h < 10) {
            sb.append("0");
        }
        sb.append(h);
        sb.append(":");
        let m = this.getUTCMinutes();
        if (m < 10) {
            sb.append("0");
        }
        sb.append(m);
        sb.append(":");
        let s = this.getUTCSeconds();
        if (s < 10) {
            sb.append("0");
        }
        sb.append(s);
        sb.append(" GMT")
        return sb.toString();
    }

    /**
     * Parses int from substring if indexes are in string
     */
    private static parseIfContains(dateStr: String, begin: int, end: int): int {
        if (dateStr.length >= end && begin < end) {
            return Double.parseInt(dateStr.substring(begin, end)) as int;
        }
        return Date.UNSUCCESSFUL_PARSING;
    }

    private static readonly UNSUCCESSFUL_PARSING: int = -1;
}
