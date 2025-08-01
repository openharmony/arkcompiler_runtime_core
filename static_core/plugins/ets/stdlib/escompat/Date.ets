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

const secPerHour: long = secondsPerMinute * minutesPerHour;

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
const maxDateOffset: long;

const INVALID_DATE: long;

// initialized in _initializerBlock_.ets
/** Day names */
const dayNames: String[];

/** Month names */
const monthNames: String[];

/** First day of a month in a year */
const firstDayInMonthNormal: int[];
/** First day of a month in a leap year */
const firstDayInMonthLeap: int[];

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
    return (time / msPerDay).toInt();
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
    let approx: int = (floor(time / msPerDay / 365.2425)).toInt() + 1970 as int;
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

    let inLeapYear = ecmaInLeapYear(year);

    for (let i = 1; i < 12; i++) {
        if (dayWithinYear < getFirstDayInMonth(inLeapYear, i)) {
            return (i - 1).toByte();
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
    let inLeapYear = ecmaInLeapYear(year);

    let month: byte = 11;

    for (let i = 1; i < 12; i++) {
        if (dayWithinYear < getFirstDayInMonth(inLeapYear, i)) {
            month = (i - 1).toByte();
            break;
        }
    }

    return (dayWithinYear + 1 - getFirstDayInMonth(inLeapYear, month)).toByte();
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
    let weekDay = ((day + 4) % 7).toByte();

    return weekDay >= 0 ? weekDay : (weekDay + 7).toByte();
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
    return (timeInDay / msPerHour).toByte();
}

/**
 * @see ECMA-262, 21.4.1.13
 *
 * @returns Minute component of a given time.
 */
function ecmaMinFromTime(time: long): byte {
    let timeInDay = ecmaTimeInDayFromTime(time);
    return ((timeInDay / msPerMinute) % minutesPerHour).toByte();
}

/**
 * @see ECMA-262, 21.4.1.13
 *
 * @returns Seconds component of a given time.
 */
function ecmaSecFromTime(time: long): byte {
    let timeInDay = ecmaTimeInDayFromTime(time);
    return ((timeInDay / msPerSecond) % secondsPerMinute).toByte();
}

/**
 * @see ECMA-262, 21.4.1.13
 *
 * @returns Milliseconds component of a given time.
 */
function ecmaMsFromTime(time: long): short {
    let timeInDay = ecmaTimeInDayFromTime(time);
    return (timeInDay % msPerSecond).toShort();
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
    let hh = trunc(hr).toLong();
    let mm = trunc(min).toLong();
    let ss = trunc(sec).toLong();
    let mi = trunc(ms).toLong();

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
    let y = trunc(year).toLong();
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
    let ym = (year + floor(month.toDouble() / 12.0 as double)).toInt();
    let mn = (month % 12).toByte();

    if (mn < 0) {
        mn += 12;
    }

    let days: long = ecmaDayFromYear(ym).toLong() + (getFirstDayInMonth(ecmaInLeapYear(ym), mn)).toLong() + (date as long) - 1;

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
    let y = trunc(year).toLong();
    let m = trunc(month).toLong();
    let d = trunc(day).toLong();

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
function ecmaTimeClip(time: long): long {
    if ((time > maxDateOffset) || (time < -maxDateOffset)) {
        throw new RangeError()
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
        floor((year - 1601) / 400.0)).toInt();
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
    return (ecmaDaysInYear(year) - dayCountInYear).toByte();
}

/**
 * Date JS API-compatible class
 */
export class Date {
    private static readonly DATE_TIME_FORMAT_OPTIONS: Intl.DateTimeFormatOptions = {
        year: "numeric",
        month: "numeric",
        day: "numeric",
        hour: "2-digit",
        minute: "2-digit",
        second: "2-digit"
    }

    private static isoDateRegExp: RegExp = new RegExp("^(-?(?:[0-9]{1,}))-(0[1-9]|1[0-2])-(0[1-9]|[12][0-9]|3[01])(?:T([01][0-9]|2[0-3])(?::[0-5][0-9])?(?::[0-5][0-9](?:\\.[0-9]+)?)?)?(Z|(?:[+-])(?:[01][0-9]|2[0-3]):[0-5][0-9])?$");

    private static readonly DATE_FORMAT_OPTIONS: Intl.DateTimeFormatOptions = {
        year: "numeric",
        month: "numeric",
        day: "numeric"
    }

    private static readonly TIME_FORMAT_OPTIONS: Intl.DateTimeFormatOptions = {
        hour: "2-digit",
        minute: "2-digit",
        second: "2-digit"
    }

    /**
     * Regular Expression to Check if a String is an ISO Date Format
     *
     * @param dateStr Date in string format
     * @returns If the string is in ISO date format, return true; otherwise, return false.
     */
    private static isIsoDate(dateStr:String): boolean {
        return Date.isoDateRegExp.test(dateStr);
    }

    /** Stored value of Date, milliseconds */
    private ms: long;
    private isValid: boolean = true;
    private get TZOffset(): long {
        return Date.getLocalTimezoneOffset(this.ms as long);
    }

    /**
     * The `parseDateFromString()` static method parses a string representation of a date,
     * and returns the number of milliseconds since January 1, 1970, 00:00:00 UTC
     * or returns `INVALID_DATE` if the string is unrecognized or, in some cases,
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
     */
    private static parseDateFromString(dateStr: String): number {
        if (!Date.isIsoDate(dateStr)) {
            let parser = new LegacyDateParser(dateStr);
            let parseDate = parser.parseDate()
            if (parseDate === INVALID_DATE) {
                return NaN;
            }
            return parseDate.toDouble();
        }
        let parseResult = new ISODateParseResult()
        try {
            parseResult = new ISODateParser(dateStr).parse()
        } catch (parseError) {
            return INVALID_DATE
        }

        const date = new Date(0)
        date.setUTCFullYear(parseResult.year);

        if (parseResult.month > 0) {
            if (parseResult.month > monthCountPerYear) {
                return INVALID_DATE
            }
            date.setUTCMonth(parseResult.month - 1)
        }

        if (parseResult.day > maxDaysInMonth) {
            return INVALID_DATE
        }
        date.setUTCDay(parseResult.day)

        if (parseResult.hour > hoursPerDay) {
            return INVALID_DATE
        }
        date.setUTCHours(parseResult.hour)

        if (parseResult.minute > minutesPerHour) {
            return INVALID_DATE
        }
        date.setUTCMinutes(parseResult.minute)

        if (parseResult.second > secondsPerMinute) {
            return INVALID_DATE
        }
        date.setUTCSeconds(parseResult.second)

        if (parseResult.ms > msPerSecond) {
            return INVALID_DATE
        }
        date.setUTCMilliseconds(parseResult.ms)

        const tzOffset = parseResult.tzOffset
        if (tzOffset !== undefined) {
            return date.valueOf() + tzOffset
        } else {
            return date.valueOf() + Date.getLocalTimezoneOffset((date.valueOf()).toLong()) * msPerMinute
        }
    }
    /**
     * Default constructor.
     *
     * @see ECMA-262, 21.4.2.1
     *
     * @description Initializes Date instance with current time.
     */
    constructor() {
        this.ms = Date.now().toLong();
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
            this.ms = (trunc((value as Date).valueOf() as number)).toLong()
        }
        else if (typeof value == "string") {
            let result = Date.parseDateFromString(value as string);
            if (Double.isNaN(result)) {
                this.isValid = false;
            }
            this.ms = result.toLong();
        }
        else {
            if (isNaN(value as number)) {
                this.ms = (maxDateOffset + 1) as long;
            } else {
                this.ms = (trunc(value as number)).toLong();
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
        return this.isValid && ms >= -maxDateOffset && ms <= maxDateOffset;
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
     */
    public valueOf(): number {
        if (!this.isDateValid()) {
            return NaN;
        }
        return this.ms.toDouble();
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
     * Gets a string with a language-sensitive representation of the time portion of the date.
     *
     * @returns a language-sensitive representation of the time portion of the date.
     */
    public toLocaleTimeString(): string {
        return this.toLocaleTimeString(undefined)
    }

    private appendDefaultTimeOptions(options: Intl.DateTimeFormatOptions): Intl.DateTimeFormatOptions {
        const appendTimeOptions = !options.dayPeriod
                && !options.hour
                && !options.minute
                && !options.second
                && !options.fractionalSecondDigits

        let effectiveOptions = options
        if (appendTimeOptions) {
            // client options instance should not be modified, so creating a copy
            effectiveOptions = new Intl.DateTimeFormatOptions(options)
            effectiveOptions.hour = Date.TIME_FORMAT_OPTIONS.hour
            effectiveOptions.minute = Date.TIME_FORMAT_OPTIONS.minute
            effectiveOptions.second = Date.TIME_FORMAT_OPTIONS.second
        }

        return effectiveOptions
    }

    /**
     * Gets a string with a language-sensitive representation of the time portion of the date with respect to locale.
     *
     * @param locale
     *
     * @returns a language-sensitive representation of the time portion of the date with respect to locale.
     */
    public toLocaleTimeString(locales?: Intl.LocalesArgument, options?: Intl.DateTimeFormatOptions): string {
        if (options && options.dateStyle) {
            throw new TypeError("Invalid option : dateStyle")
        }

        const effectiveOptions = !options ? Date.TIME_FORMAT_OPTIONS : this.appendDefaultTimeOptions(options!)
        return new Intl.DateTimeFormat(intlLocalesToLanguageTags(locales), effectiveOptions).format(this.getTime())
    }

    /**
     * Gets a string with a language-sensitive representation of this date with respect to locale.
     *
     * @param locale
     *
     * @returns a language-sensitive representation of this date.
     */
    public toLocaleString(locales?: Intl.LocalesArgument, options?: Intl.DateTimeFormatOptions): string {
        const formatter = new Intl.DateTimeFormat(intlLocalesToLanguageTags(locales), options ?? Date.DATE_TIME_FORMAT_OPTIONS)
        return formatter.format(this.getTime())
    }

    /**
     * Gets a string with a language-sensitive representation of this date.
     *
     * @returns a language-sensitive representation of this date.
     */
    public toLocaleString(): string {
        return this.toLocaleString(undefined)
    }

    /**
     * Gets a string with a language-sensitive representation
     * of the date portion of the specified date in the user agent's timezone.
     *
     * @returns a string with a language-sensitive representation
     * of the date portion of the specified date in the user agent's timezone.
     */
    public toLocaleDateString(): string {
        return this.toLocaleDateString(undefined)
    }

    private appendDefaultDateOptions(options: Intl.DateTimeFormatOptions): Intl.DateTimeFormatOptions {
        const appendDateOptions = !options.weekday && !options.year && !options.month && !options.day

        let effectiveOptions = options
        if (appendDateOptions) {
            // client options instance should not be modified, so creating a copy
            effectiveOptions = new Intl.DateTimeFormatOptions(options)
            effectiveOptions.year = Date.DATE_FORMAT_OPTIONS.year
            effectiveOptions.month = Date.DATE_FORMAT_OPTIONS.month
            effectiveOptions.day = Date.DATE_FORMAT_OPTIONS.day
        }

        return effectiveOptions
    }

    /**
     * Returns a string with a language-sensitive representation
     * of the date portion of the specified date in the user agent's timezone.
     *
     * @returns a string with a language-sensitive representation
     * of the date portion of the specified date in the user agent's timezone.
     */
    public toLocaleDateString(locales?: Intl.LocalesArgument, options?: Intl.DateTimeFormatOptions): string {
        if (options && options.timeStyle) {
            throw new TypeError("Invalid option : timeStyle")
        }

        const effectiveOptions = !options ? Date.DATE_FORMAT_OPTIONS : this.appendDefaultDateOptions(options)
        const formatter = new Intl.DateTimeFormat(intlLocalesToLanguageTags(locales), effectiveOptions)
        return formatter.format(this.getTime())
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
        let inLeapYear = ecmaInLeapYear(this.getFullYear().toInt());
        let firstDay: int = getFirstDayInMonth(inLeapYear, ecmaMonthFromTime(localTime));
        return (day - firstDay + 1).toDouble();
    }

    /**
     * Changes the day of the month of a given Date instance, based on local time.
     *
     * @param value new day.
     */
    public setDate(value: byte): void {
        let day = this.getDate();
        this.ms -= (day * msPerDay).toLong();
        this.ms += value * msPerDay;
    }

    /**
     * Changes the day of the month of a given Date instance, based on local time.
     *
     * @param value new day.
     */
    public setDate(value: number): number {
        this.setDate(value.toByte());
        return this.ms.toDouble();
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
        let inLeapYear = ecmaInLeapYear(this.getFullYear().toInt());
        let firstDay: int = getFirstDayInMonth(inLeapYear, ecmaMonthFromTime(localTime));
        return (day - firstDay + 1).toDouble();
    }

    /**
     * Changes the day of the month of a given Date instance, based on UTC time.
     *
     * @param value new day.
     */
    public setUTCDate(value: byte): void {
        let day = (this.getUTCDate()).toByte();
        this.ms -= day * msPerDay;
        this.ms += value * msPerDay;
    }

    /**
     * Changes the day of the month of a given Date instance, based on UTC time.
     *
     * @param value new day.
     */
    public setUTCDate(value: number): number {
        this.setUTCDate(value.toByte());
        return this.ms.toDouble();
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
        return this.ms.toDouble();
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
        return (ecmaWeekDay(this.ms - this.TZOffset * 60 * msPerSecond)).toDouble();
    }

    /**
     * Returns the day of the week in the specified date according to universal time, where 0 represents Sunday.
     *
     * @returns An integer number, between 0 and 6, corresponding to the day of the week
     * for the given date, according to local time: 0 for Sunday, 1 for Monday, 2 for Tuesday, and so on.
     */
    public getUTCDay(): number {
        return (ecmaWeekDay(this.ms)).toDouble();
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
        return this.getFullYear().toInt();
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
        return (ecmaYearFromTime(this.ms)).toDouble();
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
        return (ecmaYearFromTime(localTime)).toDouble();
    }

    /**
     * Sets the full year for a specified date according to universal time.
     *
     * @param value new year
     */
    public setUTCFullYear(value: number, month?: number, date?: number): number {
        this.setUTCFullYear(value.toInt());
        if (month !== undefined) {
            this.setUTCMonth(month.toInt());
        }
        if (date !== undefined) {
            this.setUTCDate(date.toByte());
        }
        return this.ms.toDouble();
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
        this.setYear(value.toInt())
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
        this.setFullYear(value.toInt());
        if (month !== undefined) {
            this.setMonth(month.toInt());
        }
        if (date !== undefined) {
            this.setDate(date.toByte());
        }
        return this.ms.toDouble();
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
        return ecmaHourFromTime(localTime).toDouble();
    }

    /**
     * Returns the hours in the specified date according to universal time.
     *
     * @returns An integer number, between 0 and 23, representing the hour for the given date according to UTC time.
     */
    public getUTCHours(): number {
        return ecmaHourFromTime(this.ms).toDouble();
    }

    /**
     * Sets the hours for a specified date according to local time.
     *
     * @param value new hours
     */
    public setHours(value: byte): void {
        if (value < 0 || value > 23) {
            throw new AssertionError("Invalid value " + value + " for \"value\", has to be in range 0..23");
        }
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
        this.setHours(value.toByte());
        if (min !== undefined) {
            this.setMinutes(min.toByte());
        }
        if (sec !== undefined) {
            this.setSeconds(sec.toByte());
        }
        if (ms !== undefined) {
            this.setMilliseconds(ms.toShort());
        }
        return this.ms.toDouble();
    }

    /**
     * Sets the hour for a specified date according to universal time.
     *
     * @param value new hours
     */
    public setUTCHours(value: byte): void {
        if (value < 0 || value > 23) {
            throw new AssertionError("Invalid value " + value + " for \"value\", has to be in range 0..23");
        }
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
        this.setUTCHours(value.toByte());
        if (min !== undefined) {
            this.setUTCMinutes(min.toByte());
        }
        if (sec !== undefined) {
            this.setUTCSeconds(sec.toByte());
        }
        if (ms !== undefined) {
            this.setUTCMilliseconds(ms.toShort());
        }
        return this.ms.toDouble();
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
        return (ecmaMsFromTime(localTime)).toDouble();
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
        return d.valueOf().toLong();
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
        return (ecmaMakeDate(ecmaMakeDay(ecmaMakeFullYear(year), 0.0, 1.0), ecmaMakeTime(0.0, 0.0, 0.0, 0.0))).toDouble()
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
        return (ecmaMakeDate(
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
        )).toDouble();
    }

    /**
     * Returns the milliseconds portion of the time object's value according to universal time.
     *
     * @returns the milliseconds portion of the time object's value according to universal time.
     */
    public getUTCMilliseconds(): number {
        let localTime = this.ms;
        return (ecmaMsFromTime(localTime)).toDouble();
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
        this.setMilliseconds(value.toShort());
        return this.ms.toDouble();
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
        this.setUTCMilliseconds(value.toShort());
        return this.ms.toDouble();
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
        return (ecmaSecFromTime(localTime)).toDouble();
    }

    /**
     * Returns the seconds in the specified date according to universal time.
     *
     * @returns the seconds in the specified date according to universal time.
     */
    public getUTCSeconds(): number {
        return (ecmaSecFromTime(this.ms)).toDouble();
    }

    /**
     * Sets the seconds for a specified date according to local time.
     *
     * @param value new seconds
     */
    public setSeconds(value: number, ms?: number): number {
        this.setSeconds(value.toByte());
        if (ms !== undefined) {
            this.setMilliseconds(ms.toShort());
        }
        return this.ms.toDouble();
    }

    /**
     * Sets the seconds for a specified date according to local time.
     *
     * @param value new seconds
     */
    public setSeconds(value: byte): void {
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
        if (value < 0 || value >= 60) {
            throw new AssertionError("Invalid value " + value + " for \"value\", has to be in range 0..60");
        }
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
        this.setUTCSeconds(value.toByte());
        if (ms !== undefined) {
            this.setUTCMilliseconds(ms.toShort());
        }
        return this.ms.toDouble();
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
        return ecmaMinFromTime(localTime).toDouble();
    }

    /**
     * Sets the minutes for a specified date according to universal time.
     *
     * @param value new minutes
     */
    public setUTCMinutes(value: byte): void {
        if (value < 0 || value >= 60) {
            throw new AssertionError("Invalid value " + value + " for \"value\", has to be in range 0..60");
        }
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
        this.setUTCMinutes(value.toByte());
        if (sec !== undefined) {
            this.setUTCSeconds(sec.toByte());
        }
        if (ms !== undefined) {
            this.setUTCMilliseconds(ms.toShort());
        }
        return this.ms.toDouble();
    }

    /**
     * Returns the minutes in the specified date according to universal time.
     *
     * @returns the minutes in the specified date according to universal time.
     */
    public getUTCMinutes(): number {
        return ecmaMinFromTime(this.ms).toDouble();
    }

    /**
     * Sets the minutes for a specified date according to local time.
     *
     * @param value new minutes
     */
    public setMinutes(value: byte): void {
        if (value < 0 || value >= 60) {
            throw new AssertionError("Invalid value " + value + " for \"value\", has to be in range 0..60");
        }
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
        this.setMinutes(value.toByte());
        if (sec !== undefined) {
            this.setSeconds(sec.toByte());
        }
        if (ms !== undefined) {
            this.setMilliseconds(ms.toShort());
        }
        return this.ms.toDouble();
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
        return ecmaMonthFromTime(localTime).toDouble();
    }

    /**
     * Returns the month of the specified date according to universal time, as a zero-based value (where zero indicates the first month of the year).
     *
     * @returns  An integer number, between 0 and 11, representing the month in the given date according to UTC time.
     * 0 corresponds to January, 1 to February, and so on.
     */
    public getUTCMonth(): number {
        return ecmaMonthFromTime(this.ms).toDouble();
    }

    /**
     * Sets the month for a specified date according to the currently set year.
     *
     * @param month new month
     */
    public setMonth(month: number, date?: number): number {
        this.setMonth(month.toInt());
        if (date !== undefined) {
            this.setDate(date.toByte());
        }
        return this.ms.toDouble();
    }

    /**
     * Sets the month for a specified date according to the currently set year.
     *
     * @param month new month
     */
    public setMonth(month: int): void {
        let inLeapYear = ecmaInLeapYear(this.getFullYear().toInt());
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
        this.setUTCMonth(month.toInt());
        if (date !== undefined) {
            this.setUTCDate(date.toByte());
        }
        return this.ms.toDouble();
    }

    /**
     * Sets the month for a specified date according to universal time.
     *
     * @param month new month
     */
    public setUTCMonth(month: int): void {
        let inLeapYear = ecmaInLeapYear(this.getFullYear().toInt());
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
        this.setTime(value.toLong());
        return this.ms.toDouble();
    }

    /**
     * Parses a string representation of a date,
     * and returns the number of milliseconds since January 1, 1970, 00:00:00 UTC
     * or returns `INVALID_DATE` if the string is unrecognized or, in some cases,
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
     * If the argument doesn't represent a valid date, `INVALID_DATE` value is returned.
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
        if (!this.isValid) {
            return NaN;
        }
        return this.TZOffset.toDouble();
    }

    /**
     * Sets the difference, in minutes, between a date as
     * evaluated in the UTC time zone, and the same date as evaluated in the local time zone.
     *
     * @param value new timezone offset
     */
    public setTimezoneOffset(value: number): number {
        return this.setTimezoneOffset(value.toInt()) as number
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
        if (!this.isDateValid()) {
            throw new RangeError('Invalid time value');
        }
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
        sb.append((abs(y)).toLong());
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
    public toJSON(): string | null {
        if (!this.isDateValid()) {
            return null;
        }
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
        sb.append(dayNames[this.getDay().toInt()]);
        sb.append(" ");
        sb.append(monthNames[this.getMonth().toInt()]);
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
        sb.append((abs(y)).toLong());
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
        let res: String = "Invalid Date";
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
        sb.append(dayNames[this.getUTCDay().toInt()]);
        sb.append(", ");
        let d = this.getUTCDate();
        if (d < 10) {
            sb.append("0");
        }
        sb.append(d);
        sb.append(" ");
        sb.append(monthNames[this.getUTCMonth().toInt()]);
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
        sb.append(y.toLong());
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
}

class ISODateParseResult {
    year: int = 0
    month: int = 0
    day: int = 0
    hour: int = 0
    minute: int = 0
    second: int = 0
    ms: int = 0
    tzOffset: long | undefined = undefined

    toString(): string {
        return `{ year: ${this.year}, month: ${this.month}, day: ${this.day}, hour: ${this.hour}, minute: ${this.minute}, second: ${this.second}, ms: ${this.ms}, TZ offset: ${this.tzOffset} }`
    }
}

const YEAR_COMPONENT_LENGTH: int = 4
const EXPANDED_YEAR_COMPONENT_LENGTH: int = 6
const MONTH_COMPONENT_LENGTH: int = 2
const DAY_COMPONENT_LENGTH: int = 2
const HOUR_COMPONENT_LENGTH: int = 2
const MINUTE_COMPONENT_LENGTH: int = 2
const SECOND_COMPONENT_LENGTH: int = 2
const MS_COMPONENT_LENGTH: int = 3

const DATE_TIME_SEPARATOR: string = "T"
const DATE_COMPONENTS_SEPARATOR: string = "-"
const TIME_COMPONENTS_SEPARATOR: string = ":"
const FRACTIONAL_SECONDS_SEPARATOR: string = "."

final class ISODateParseError extends Error {}

// https://tc39.es/ecma262/multipage/numbers-and-dates.html#sec-date-time-string-format
class ISODateParser {
    private readonly dateStr: string
    private currentPos: int = 0
    // limitPos
    private readonly endPos: int

    constructor(dateStr: string) {
        this.dateStr = dateStr
        this.endPos = dateStr.length.toInt()
    }

    public parse(): ISODateParseResult {
        const parseResult = this.parseDate()

        // end-of-text check
        if (this.currentPos != this.endPos) {
            throw new ISODateParseError()
        }

        return parseResult
    }

    public parseDate(): ISODateParseResult {
        const result = new ISODateParseResult()

        if (this.match("+")) {
            // expanded year +YYYYYY
            result.year = this.parseInt(EXPANDED_YEAR_COMPONENT_LENGTH)
        } else if (this.match("-")) {
            // expanded year -YYYYYY
            result.year = -this.parseInt(EXPANDED_YEAR_COMPONENT_LENGTH)
        } else {
            result.year = this.parseInt(YEAR_COMPONENT_LENGTH)
        }

        if (!this.match(DATE_COMPONENTS_SEPARATOR)) {
            return result
        }

        result.month = this.parseInt(MONTH_COMPONENT_LENGTH)
        if (!this.match(DATE_COMPONENTS_SEPARATOR)) {
            return result
        }

        result.day = this.parseInt(DAY_COMPONENT_LENGTH)
        if (!this.match(DATE_TIME_SEPARATOR)) {
            // ISO date only TZ = UTC
            result.tzOffset = 0
            return result
        }

        result.hour = this.parseInt(HOUR_COMPONENT_LENGTH)
        if (!this.match(TIME_COMPONENTS_SEPARATOR)) {
            throw new ISODateParseError()
        }

        result.minute = this.parseInt(MINUTE_COMPONENT_LENGTH)
        if (this.match(TIME_COMPONENTS_SEPARATOR)) {
            result.second = this.parseInt(SECOND_COMPONENT_LENGTH)
            if (this.match(FRACTIONAL_SECONDS_SEPARATOR)) {
                result.ms = this.parseMillis()
            }
        }

        if (this.match("Z")) {
            result.tzOffset = 0
            return result
        } else if (this.match("+")) {
            result.tzOffset = -this.parseTimeZoneOffset()
            return result
        } else if (this.match("-")) {
            result.tzOffset = this.parseTimeZoneOffset()
            return result
        }

        // local timezone case
        return result
    }

    private parseTimeZoneOffset(): long {
        const tzOffsetHour = this.parseInt(HOUR_COMPONENT_LENGTH)
        if (!this.match(TIME_COMPONENTS_SEPARATOR)) {
            throw new ISODateParseError()
        }

        const tzOffsetMinute = this.parseInt(MINUTE_COMPONENT_LENGTH)

        return tzOffsetHour * msPerHour + tzOffsetMinute * msPerMinute
    }

    private parseInt(length: int): int {
        const startPos = this.currentPos
        const endPos = this.currentPos + length

        if (endPos > this.endPos) {
            throw new ISODateParseError()
        }

        const literalText = this.dateStr.substring(startPos, endPos)
        for (const literalChar of literalText.getChars()) {
            if (!Char.isDecDigit(literalChar)) {
                throw new ISODateParseError()
            }
        }

        const literalValue = Double.parseInt(literalText).toInt()
        this.currentPos += length

        return literalValue
    }

    private parseMillis(): int {
        const startPos = this.currentPos

        let currentPos = startPos
        while (currentPos < this.endPos && Char.isDecDigit(this.dateStr.charAt(currentPos))) {
            currentPos++
        }

        const millisTextLength = currentPos - startPos
        if (millisTextLength == 0) {
            throw new ISODateParseError()
        }

        const millisText = (millisTextLength > MS_COMPONENT_LENGTH)
                ? this.dateStr.substring(startPos, startPos + MS_COMPONENT_LENGTH)
                : this.dateStr.substring(startPos, currentPos).padEnd(MS_COMPONENT_LENGTH, "0")

        const millisValue = Double.parseInt(millisText).toInt()

        this.currentPos = currentPos
        return millisValue
    }

    private match(expectedToken: string): boolean {
        const expectedTokenLength = expectedToken.length.toInt()
        if (this.currentPos + expectedTokenLength > this.endPos) {
            return false
        }

        const actualToken = this.dateStr.substring(this.currentPos, this.currentPos + expectedTokenLength)
        if (actualToken == expectedToken) {
            this.currentPos += actualToken.length.toInt()
            return true
        }

        return false
    }
}

class ReadDataResult {
    value: int;
    strValue: string = '';
    length: int;

    constructor(value: int, length: int) {
        this.value = value;
        this.length = length;
    }

    constructor(value: string, length: int) {
        this.strValue = value;
        this.length = length;
    }
}

class DateStringReader {
    private static maxDecimal = (Int.MAX_VALUE - 9) / 10;
    private data: string;
    private index: int = 0;
    private length: int = 0;
    private value: string = '\0';

    constructor(str: string) {
        this.data = str;
        this.length = str.length.toInt();
        this.nextChar();
    }

    public nextChar(): void {
        this.value = this.index < this.length ? this.data[this.index] : '\x7F';
        this.index++;
    }

    public getIndex(): int {
        return this.index;
    }

    public readNumber(): ReadDataResult {
        let length:int = 0;
        let num:int = 0;
        while (Char.isDecDigit(this.value.charAt(0))) {
            num = parseInt(this.value).toInt() + (num * 10);
            length++;
            if (num > DateStringReader.maxDecimal) {
                break;
            }
            this.nextChar();
        }
        while (Char.isDecDigit(this.value.charAt(0))) {
            this.nextChar();
        }
        return new ReadDataResult(num, length);
    }

    public readAlphabet(size: int): ReadDataResult {
        let sb =  new StringBuilder();
        let length = 0;
        while (Char.isLetter(this.value.charAt(0))) {
            if (length < size) {
                sb.append(this.value.toLowerCase());
            }
            length++;
            this.nextChar();
        }
        return new ReadDataResult(sb.toString(), length);
    }

    public getValue(): string {
        return this.value;
    }
}

enum DateValueType {
    DATE_INVALID,
    DATE_UNKNOWN,
    DATE_NUMBER,
    DATE_SYMBOL,
    DATE_SPACE,
    DATE_STRING_END,
    DATE_TIME_ZONE,
    DATE_TIME_FALG,
    DATE_MONTH,
    DATE_INVALID_WORD,
    DATE_WORD_START = DATE_TIME_ZONE,
}

class DateUnit {
    private type: DateValueType;
    private value: int;
    private length: int;

    constructor(type: DateValueType, value: int, length: int) {
        this.type = type;
        this.value = value;
        this.length = length;
    }

    static numericValue(value: int, length: int): DateUnit {
        return new DateUnit(DateValueType.DATE_NUMBER, value, length);
    }

    static symbolValue(ch: string): DateUnit {
        return new DateUnit(DateValueType.DATE_SYMBOL, ch.charCodeAt(0), 1);
    }

    static word(type: DateValueType, value: int, length: int): DateUnit {
        return new DateUnit(type, value, length);
    }

    static space(): DateUnit {
        return new DateUnit(DateValueType.DATE_SPACE, 0, 1);
    }

    static stringEnd(): DateUnit {
        return new DateUnit(DateValueType.DATE_STRING_END, 0, 0);
    }

    static invalid(): DateUnit {
        return new DateUnit(DateValueType.DATE_INVALID, 0, 0);
    }

    static unknown(): DateUnit {
        return new DateUnit(DateValueType.DATE_UNKNOWN, 0, 1);
    }

    isInvalid(): boolean {
        return this.type === DateValueType.DATE_INVALID;
    }

    isUnknown(): boolean {
        return this.type === DateValueType.DATE_UNKNOWN;
    }

    isNumberType(): boolean {
        return this.type === DateValueType.DATE_NUMBER;
    }

    isSymbolType(): boolean {
        return this.type === DateValueType.DATE_SYMBOL;
    }

    isSymbolChar(ch: string): boolean {
        return this.type === DateValueType.DATE_SYMBOL && this.value === ch.charCodeAt(0);
    }

    isStringEnd(): boolean {
        return this.type === DateValueType.DATE_STRING_END;
    }

    isTimeZoneType(): boolean {
        return this.type === DateValueType.DATE_TIME_ZONE;
    }

    isTimeFlag(): boolean {
        return this.type === DateValueType.DATE_TIME_FALG;
    }

    isInvalidWord(): boolean {
        return this.type === DateValueType.DATE_INVALID_WORD;
    }

    isMonth(): boolean {
        return this.type === DateValueType.DATE_MONTH;
    }

    isWord(): boolean {
        return this.type >= DateValueType.DATE_TIME_ZONE;
    }

    isSign(): boolean {
        return this.isSymbolType() && (this.value === '-'.charCodeAt(0) || this.value === '+'.charCodeAt(0));
    }

    isSixDecimalDigit(): boolean {
        return this.type === DateValueType.DATE_NUMBER && this.length === 6;
    }

    isFourDecimalDigit(): boolean {
        return this.type === DateValueType.DATE_NUMBER && this.length === 4;
    }

    isTwoDecimalDigit(): boolean {
        return this.type === DateValueType.DATE_NUMBER && this.length === 2;
    }

    isWordZ(): boolean {
        return this.type === DateValueType.DATE_TIME_ZONE && this.value === 0;
    }

    isSpaceOrTab(): boolean {
        return this.type === DateValueType.DATE_SPACE;
    }

    isValidFinallyTime(): boolean {
        return this.isStringEnd() || this.isSign() || this.isWordZ() || this.isSpaceOrTab();
    }

    getValue(): int {
        return this.value;
    }

    getType(): DateValueType {
        return this.type;
    }

    getLength(): int {
        return this.length;
    }
}

class DateLexer {
    private str: DateStringReader;
    private date: DateUnit;

    constructor(str: DateStringReader) {
      this.str = str;
      this.date = this.read();
    }

    getDate(): DateUnit {
      return this.date;
    }

    nextDate(): DateUnit {
      const current = this.date;
      this.date = this.read();
      return current;
    }

    ignoreSymbol(ch: string): boolean {
      if (!this.date.isSymbolChar(ch)) {
        return false;
      }
      this.date = this.read();
      return true;
    }

    private read(): DateUnit {
      if (Char.isDecDigit(this.str.getValue().charAt(0))) {
        const res  = this.str.readNumber();
        return DateUnit.numericValue(res.value, res.length);
      }
      if (this.str.getValue() == '\x7F') {
        return DateUnit.stringEnd();
      }
      if (this.str.getValue() == ':') {
        this.str.nextChar();
        return DateUnit.symbolValue(':');
      }
      if (this.str.getValue() == '+') {
        this.str.nextChar();
        return DateUnit.symbolValue('+');
      }
      if (this.str.getValue() == '-') {
        this.str.nextChar();
        return DateUnit.symbolValue('-');
      }
      if (this.str.getValue() == '.') {
        this.str.nextChar();
        return DateUnit.symbolValue('.');
      }
      if (Char.isLetter(this.str.getValue().charAt(0))) {
        const alphabet = this.str.readAlphabet(3);
        const minLen: int = Math.min(alphabet.length, 3).toInt();
        const str = alphabet.strValue.substring(0, minLen);
        let res = 0;
        let type = DateValueType.DATE_INVALID_WORD;
        if (str === 't') {
            type = DateValueType.DATE_TIME_FALG;
        }
        if (str === 'z') {
            type =  DateValueType.DATE_TIME_ZONE;
        }
        if (str === 'utc' || str === 'gmt') {
            res = 1;
            type = DateValueType.DATE_TIME_ZONE;
        }
        const monthNames = ['jan', 'feb', 'mar', 'apr', 'may', 'jun', 'jul', 'aug', 'sep', 'oct', 'nov', 'dec'];
        const index = monthNames.indexOf(str.toLowerCase());
        if (index !== -1) {
            res = index.toInt() + 1;
            type = DateValueType.DATE_MONTH;
        }
        return DateUnit.word(type, res, alphabet.length);
      }
      if (Char.isWhiteSpace(this.str.getValue().charAt(0))) {
        this.str.nextChar();
        return DateUnit.space();
      }
      this.str.nextChar();
      return DateUnit.unknown();
    }
}

class DayValue {
    data: int[] = [0, 0, 0];
    index: int = 0;
    month: int = Int.MAX_VALUE;

    setData(data: int): boolean {
        if (this.index < 3) {
            this.data[this.index++] = data;
            return true;
        }
        return false;
    }

    setMonth(month: int): void {
        this.month = month;
    }

    static monthIsValid(n: int): boolean {
        return n >= 1 && n <= monthCountPerYear;
    }

    static dayIsValid(n: int): boolean {
        return n >= 1 && n <= maxDaysInMonth;
    }

    isFull(): boolean {
        return this.index === 3;
    }

    getIndex(): int {
        return this.index;
    }
}

class TimeValue {
  hour_: int = 0
  minute_: int = 0
  second_: int = 0
  ms_: int = 0;
  index: int = 0;

  setData(data: int): boolean {
    if (this.index < 4) {
        switch (this.index) {
            case 0:
                this.hour_ = data;
                break;
            case 1:
                this.minute_ = data;
                break;
            case 2:
                this.second_ = data;
                break;
            case 3:
                this.ms_ = data;
                break;
            default:
                return false;
            }
        this.index++;
        return true;
    }
    return false;
  }

  static minuteIsValid(n: int): boolean {
    return n >= 0 && n <= 59;
  }

  static secondIsValid(n: int): boolean {
    return n >= 0 && n <= 59;
  }

  static hourIsValid(n: int): boolean {
    return n >= 0 && n <= 24;
  }

  static milliSecondIsValid(n: int): boolean {
    return n >= 0 && n <= 999;
  }

  static normMilliSecond(sec: DateUnit): int {
    const len = sec.getLength();
    const value = sec.getValue();
    if (len === 3) {
      return value;
    }
    if (len === 2) {
      return value * 10;
    }
    if (len === 1) {
      return value * 100;
    }
    let divisor = 1;
    let remainingLen = len;
    while (remainingLen > 3) {
      divisor *= 10;
      remainingLen--;
    }
    return Math.floor(value / divisor).toInt();
  }

  getIndex(): int {
    return this.index;
  }

  isValid(n: int): boolean {
    return (this.index === 1 && TimeValue.minuteIsValid(n)) || (this.index === 2 && TimeValue.secondIsValid(n));
  }

  isValidSecond(n: int): boolean {
    return this.index === 2 && TimeValue.secondIsValid(n);
  }
}
class TimeZone {
  sign: int = Int.MAX_VALUE;
  hour: int = Int.MAX_VALUE;
  minute: int = Int.MAX_VALUE;

  setSign(sign: int): void {
    this.sign = sign;
  }

  setHour(hour: int): void {
    this.hour = hour;
  }

  setMin(minute: int): void {
    this.minute = minute;
  }

  setUTC(): void {
    this.sign = 1;
    this.hour = 0;
    this.minute = 0;
  }

  isUTC(): boolean {
    return this.hour === 0 && this.minute === 0;
  }

  IsLocal(): boolean {
    return this.hour === Int.MAX_VALUE;
  }

  hasSign(): boolean {
    return this.sign !== Int.MAX_VALUE;
  }

  hasHour(): boolean {
    return this.hour !== Int.MAX_VALUE;
  }

  hasMinute(): boolean {
    return this.minute !== Int.MAX_VALUE;
  }

  isValid(): boolean {
    if (!this.hasSign() && !this.hasHour() && !this.hasMinute()) {
      return true;
    }
    if (this.hasSign()) {
      if (!this.hasHour()) {
        return false;
      }
      
      if (this.hour < 0 || this.hour > 23) {
        return false;
      }
      if (this.hasMinute() && (this.minute < 0 || this.minute > 59)) {
        return false;
      }
    }
    return true;
  }

}

enum TimeField {
    YEAR = 0,
    MONTH = 1,
    DAYS = 2,
    HOUR = 3,
    MIN = 4,
    SEC = 5,
    MS = 6,
    TIMEZONE = 8,
    DATE_LENGTH = 9,
}

class LegacyDateResult { 
    private dayValue: DayValue;
    private timeValue: TimeValue;
    private timeZone: TimeZone;
    year_: int = 0;
    month_: int = 0;
    day_: int = 0;
    hour_: int = 0;
    minute_: int = 0;
    second_: int = 0;
    ms_: int = 0;
    tz_: int = 0;

    set hour(value: int) {
        if (value < 0 || value > 23) {
            throw new RangeError("Invalid hour");
        }
        this.hour_ = value;
    }

    set minute(value: int) {
        if (value < 0 || value > 59) {
            throw new RangeError("Invalid minute");
        }
        this.minute_ = value;
    }

    set second(value: int) {
        if (value < 0 || value > 59) {
            throw new RangeError("Invalid second");
        }
        this.second_ = value;
    }

    set ms(value: int) {
        if (value < 0 || value > 999) {
            throw new RangeError("Invalid ms");
        }
        this.ms_ = value;
    }

    constructor(dayValue: DayValue, timeValue: TimeValue, timeZone:TimeZone) {
        this.dayValue = dayValue;
        this.timeValue = timeValue;
        this.timeZone = timeZone;
    }

    setTimeZone() {
        if (!this.timeZone.IsLocal()) {
            if (!this.timeZone.isValid()) {
                throw new RangeError("Invalid time zone");
            }
            this.tz_ = (this.timeZone.sign * (this.timeZone.hour * secPerHour + this.timeZone.minute * secondsPerMinute)).toInt();
        } else {
            this.tz_ = Int.MAX_VALUE;
        }
    }

    setTimeValue() {
        this.hour = this.timeValue.hour_;
        this.minute = this.timeValue.minute_;
        this.second = this.timeValue.second_;
        this.ms = this.timeValue.ms_;
        if (this.hour_ === 24) {
            let rs = this.minute_ === 0 && this.second_ === 0 && this.ms_ === 0
            if (!rs) {
                throw new RangeError("Invalid TimeValue");   
            }
        }
    }

    setDayValue() {
        if (this.dayValue.index === 0) {
            throw new RangeError("Invalid DateValue");
        }
        let year:int = 1;
        let month:int = 1;
        let day:int = 1;
        for (let i = this.dayValue.index; i < 3; i++) {
            this.dayValue.data[i] = 1;
        }
        if (this.dayValue.month === Int.MAX_VALUE) {
            if ((this.dayValue.isFull() && DayValue.dayIsValid(this.dayValue.data[2]) && !DayValue.monthIsValid(this.dayValue.data[0]))) {
                year = this.dayValue.data[TimeField.YEAR];
                month = this.dayValue.data[TimeField.MONTH];
                day = this.dayValue.data[TimeField.DAYS];
            } else if (this.dayValue.isFull()) {
                year = this.dayValue.data[2];
                month = this.dayValue.data[0];
                day = this.dayValue.data[1];
            } else {
                year = this.dayValue.data[0];
                month = this.dayValue.data[1];
                day = this.dayValue.data[2];
            }
        } else {
            month = this.dayValue.month;
            if (this.dayValue.isFull()) {
                throw new RangeError("Invalid DateValue");
            }
            if (this.dayValue.index === 2) {
                if (!DayValue.dayIsValid(this.dayValue.data[0])) {
                    year = this.dayValue.data[0];
                    day = this.dayValue.data[1];
                } else {
                    day = this.dayValue.data[0];
                    year = this.dayValue.data[1];
                }
            } else {
                day = this.dayValue.data[0];
            }
        }
        if (year >= 0 && year <= 49) {
            year += 2000;
        } else if (year >= 50 && year <= 99) {
            year += 1900;
        }
        if (!DayValue.monthIsValid(month) || !DayValue.dayIsValid(day)) {
            throw new RangeError("Invalid DateValue");
        }
        this.year_ = year;
        this.month_ = month - 1; 
        this.day_ = day;
    }
}

class LegacyDateParser {
    private readonly dateStr: string

    constructor(dateStr: string) {
        this.dateStr = dateStr
    }

    private parseLegacyDateString(dateStr: string): LegacyDateResult | null {
        const reader = new DateStringReader(dateStr);
        const dateLexer = new DateLexer(reader);
        const dayValue = new DayValue();
        const timeValue = new TimeValue();
        const timeZone = new TimeZone();
        const result = this.parseLegacyDates(dateLexer, dayValue, timeValue, timeZone);
        if (!result) {
            return null;
        }
        let legacyDateResult = new LegacyDateResult(dayValue, timeValue, timeZone)
        try { 
            legacyDateResult.setTimeZone();
            legacyDateResult.setTimeValue();
            legacyDateResult.setDayValue();
        } catch (e) {
            return null
        }
        return legacyDateResult;
    }

    private parseLegacyDates(dateLexer: DateLexer, dayValue: DayValue, timeValue: TimeValue, timeZone: TimeZone) : boolean {
        let date = dateLexer.nextDate();
        let hasNumber = (dayValue.getIndex() > 0);
        while (!date.isStringEnd()) {
            if (date.isNumberType()) {
                hasNumber = true;
                if (!this.parseLegacyDateIsNumber(date, dateLexer, dayValue, timeValue)) {
                    return false;
                }
            } else if (date.isMonth()) {
                dayValue.setMonth(date.getValue());
            } else if (date.isTimeZoneType() && hasNumber) {
                timeZone.setUTC();
            } else if (date.isTimeFlag() || date.isInvalidWord()) {
                if (hasNumber) {
                    return false;
                }
                if (dateLexer.getDate().isNumberType()) {
                    return false;
                }
            } else if (date.isSign() && ((timeValue.getIndex() > 0) || timeZone.isUTC())) {
                if (!this.parseLegacyDateIsSign(date, dateLexer, timeZone)) {
                    return false;
                }
            }
            date = dateLexer.nextDate();
        }
        return true;
    }

    private parseLegacyDateIsNumber(date: DateUnit, dateLexer: DateLexer, dayValue: DayValue, timeValue: TimeValue): boolean {
        const num = date.getValue();
        // first parse as time "hh:" or "mm:"
        if (dateLexer.getDate().isSymbolChar(':')) {
            // skip ':'
            dateLexer.nextDate();
            if (!dateLexer.getDate().isNumberType()) {
                return false;
            }
            if (!timeValue.setData(num)) {
                return false;
            }
        } else if (dateLexer.getDate().isSymbolChar('.') && timeValue.isValidSecond(num)) {
            // skip '.'
            dateLexer.nextDate();
            timeValue.setData(num);
            const milliSec = dateLexer.getDate();
            if (!milliSec.isNumberType()) {
                return false;
            }
            timeValue.setData(TimeValue.normMilliSecond(milliSec).toInt());
            // ship "sss"
            dateLexer.nextDate();
            if (!dateLexer.getDate().isValidFinallyTime()) {
                return false;
            }
        } else if (timeValue.isValid(num)) {
            timeValue.setData(num);
            if (!dateLexer.getDate().isValidFinallyTime()) {
                return false;
            }
        } else if (!dayValue.setData(num)) {
            return false;
        }
        return true;
    }

    private parseLegacyDateIsSign(date: DateUnit, dateLexer: DateLexer, timeZone: TimeZone): boolean {
        if (date.isSymbolChar('-')) {
          timeZone.setSign(-1);
        } else {
          timeZone.setSign(1);
        }
        const timeNumUnit = dateLexer.getDate();
        if (!timeNumUnit.isNumberType()) {
          return false;
        }
        const timeNum = timeNumUnit.getValue();
        const numLength = timeNumUnit.getLength();
        dateLexer.nextDate();
        // parse +hh:mm
        if (dateLexer.getDate().isSymbolChar(':')) {
            // skip ':'
            dateLexer.nextDate();
            if (!dateLexer.getDate().isNumberType()) {
                return false;
            }
            timeZone.setHour(timeNum);
            timeZone.setMin(dateLexer.getDate().getValue());
            dateLexer.nextDate();
            // 2: hour length
        } else if (numLength === 1 || numLength === 2) {
            // parse GMT+hh
            timeZone.setHour(timeNum);
            timeZone.setMin(0);
            // 3,4:"GMT+hhmm" hhmm length
        } else if (numLength === 3 || numLength === 4) {
            // parse GMT+hhmm
            timeZone.setHour(Math.floor(timeNum / 100).toInt());
            timeZone.setMin(timeNum % 100);
        } else {
            return false;
        }
        return true;
    }

    private getLocalOffsetFromOS(timeMs: long, isLocal: boolean): double  {
        if (!isLocal) {
            return 0;
        }
        const date = new Date(timeMs);
        const localOffsetMinutes = date.getTimezoneOffset();
        return -localOffsetMinutes;
    }

    public parseDate(): long {
        const time = this.parseLegacyDateString(this.dateStr);
        if (time != null) {
            let day = ecmaMakeDay(time.year_, time.month_, time.day_)
            let dateTime = ecmaMakeTime(time.hour_, time.minute_, time.second_, time.ms_)
            let timeValue = ecmaTimeClip(ecmaMakeDate(day, dateTime));
            if (isNaN(timeValue)) {
                return timeValue;
            }
            let localMs:Double = 0;
            if (time.tz_ == Int.MAX_VALUE) {
                localMs = this.getLocalOffsetFromOS(timeValue, true) * msPerMinute;
            } else {
                localMs = time.tz_ * msPerSecond;
            }
            timeValue -= localMs;
            return timeValue;
        }
        return INVALID_DATE;
    }
}
