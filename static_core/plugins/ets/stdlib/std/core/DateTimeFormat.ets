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

package std.core;

export namespace Intl {
    export class DateTimeFormat {
        private static readonly DEFAULT_OPTIONS: DateTimeFormatOptions = {
            year: "numeric",
            month: "numeric",
            day: "numeric"
        }

        private readonly locale: string
        private readonly options: DateTimeFormatOptions
        private readonly pattern: string

        public constructor(locales?: string | string[], options?: DateTimeFormatOptions) {
            if (locales === undefined) {
                this.locale = Locale.defaultTag()
            } else {
                const lookupLocale = options?.localeMatcher == "lookup"
                this.locale = lookupLocale ? intlLookUpLocale(locales!) : intlBestFitLocale(locales!)
            }

            if (options === undefined || DateTimeFormatOptions.isEmpty(options)) {
                this.options = DateTimeFormat.DEFAULT_OPTIONS
            } else {
                this.options = this.getResolvedOptions(options!)
            }

            this.pattern = this.getPattern()
        }

        public static $_invoke(locales?: string | string[], options?: DateTimeFormatOptions): DateTimeFormat {
            return new DateTimeFormat(locales, options)
        }

        public format(date?: Date | number): string {
            if (date === undefined) {
                return this.formatImpl(Date.now())
            }
            if (date instanceof Date) {
                return this.formatImpl(date.getTime())
            }
            return this.formatImpl(date!)
        }

        public formatToParts(date?: Date | number): DateTimeFormatPart[] {
            if (date === undefined) {
                return this.formatToPartsImpl(Date.now())
            }
            if (date instanceof Date) {
                return this.formatToPartsImpl(date.getTime())
            }
            return this.formatToPartsImpl(date!)
        }

        public formatRange(startDate: Date | number, endDate: Date | number): string {
            const startDateTime = this.getDateTime(startDate)
            const endDateTime = this.getDateTime(endDate)
            return this.formatRangeImpl(startDateTime, endDateTime)
        }

        public formatRangeToParts(startDate: Date | number, endDate: Date | number): DateTimeRangeFormatPart[] {
            const startDateTime = this.getDateTime(startDate)
            const endDateTime = this.getDateTime(endDate)
            return this.formatRangeToPartsImpl(startDateTime, endDateTime)
        }

        public resolvedOptions(): ResolvedDateTimeFormatOptions {
            const resolvedOptions = this.resolvedOptionsImpl()

            if (resolvedOptions.hourCycle === undefined) {
                const hourCycleOpt = this.options.hourCycle
                resolvedOptions.hourCycle = hourCycleOpt ? hourCycleOpt : this.getLocaleHourCycle()
            }

            resolvedOptions.hour12 = resolvedOptions.hourCycle == "h11" || resolvedOptions.hourCycle == "h12"

            resolvedOptions.dateStyle = this.options.dateStyle
            resolvedOptions.day = this.options.day
            resolvedOptions.dayPeriod = this.options.dayPeriod
            resolvedOptions.era = this.options.era
            resolvedOptions.fractionalSecondDigits = this.options.fractionalSecondDigits
            resolvedOptions.hour = this.options.hour
            resolvedOptions.minute = this.options.minute
            resolvedOptions.month = this.options.month
            resolvedOptions.second = this.options.second
            resolvedOptions.timeStyle = this.options.timeStyle
            resolvedOptions.timeZoneName = this.options.timeZoneName
            resolvedOptions.weekday = this.options.weekday
            resolvedOptions.year = this.options.year

            return resolvedOptions
        }

        public static supportedLocalesOf(locales: string | Locale | ReadonlyArray<string | Locale>, options?: DateTimeFormatOptions): string[] {
            const langTags = intlLocalesToLanguageTags(locales)

            if (options) {
                return (options.localeMatcher == "best fit")
                        ? intlBestFitLocales(langTags)
                        : intlLookUpLocales(langTags)
            }

            return intlBestFitLocales(langTags)
        }

        private native formatImpl(timestamp: number): string

        private native formatToPartsImpl(timestamp: number): Intl.DateTimeFormatPart[]

        private native formatRangeImpl(start: number, end: number): string

        private native formatRangeToPartsImpl(start: number, end: number): Intl.DateTimeRangeFormatPart[]

        private native getLocaleHourCycle(): HourCycle | undefined

        private native resolvedOptionsImpl(): ResolvedDateTimeFormatOptions

        private getDateTime(date: Date | number): number {
            return date instanceof Date ? date.getTime() : date
        }

        private getResolvedOptions(options: DateTimeFormatOptions): DateTimeFormatOptions {
            if (options.hour12 !== undefined) {
                let hourCycle = this.getLocaleHourCycle()

                const hour12: boolean = options.hour12!
                if (hour12) {
                    if (hourCycle == "h23") {
                        hourCycle = "h11"
                    } else if (hourCycle == "h24") {
                        hourCycle = "h12"
                    }
                } else {
                    hourCycle = "h23"   
                }

                options.hourCycle = hourCycle
            }

            return options
        }

        private getHourSymbol(): string {
            const hourCycle = this.options.hourCycle
            if (hourCycle) {
                switch (hourCycle) {
                    case "h11":
                        return "K"
                    case "h12":
                        return "h"
                    case "h23":
                        return "H"
                    case "h24":
                        return "k"
                    default:
                        throw new RangeError("Invalid hour cycle: " + hourCycle)
                }
            } else {
                return "j"
            }
        }

        private getPattern(): string {
            const datePattern = this.getDatePattern()
            const timePattern = this.getTimePattern()
            return datePattern + timePattern
        }

        private getDatePattern(): string {
            const datePattern = new StringBuilder()

            this.appendYearPattern(datePattern)
            this.appendMonthPattern(datePattern)
            this.appendWeekdayPattern(datePattern)
            this.appendDayPattern(datePattern)
            this.appendDayPeriodPattern(datePattern)
            this.appendEraPattern(datePattern)

            return datePattern.toString()
        }

        private appendYearPattern(pattern: StringBuilder): void {
            const year = this.options.year
            if (year) {
                switch (year) {
                    case "numeric":
                        pattern.append("yyyy")
                        break
                    case "2-digit":
                        pattern.append("yy")
                        break
                }
            }
        }

        private appendMonthPattern(pattern: StringBuilder): void {
            const month = this.options.month
            if (month) {
                switch (month) {
                    case "numeric":
                        pattern.append("M")
                        break
                    case "narrow":
                        pattern.append("MMMMM")
                        break
                    case "long":
                        pattern.append("MMMM")
                        break
                    case "short":
                        pattern.append("MMM")
                        break
                    case "2-digit":
                        pattern.append("MM")
                        break
                }
            }
        }

        private appendWeekdayPattern(pattern: StringBuilder): void {
            const weekday = this.options.weekday
            if (weekday) {
                switch (weekday) {
                    case "long":
                        pattern.append("EEEE")
                        break
                    case "short":
                        pattern.append("EEE")
                        break
                    case "narrow":
                        pattern.append("EEEEE")
                        break
                }
            }
        }

        private appendDayPattern(pattern: StringBuilder): void {
            const day = this.options.day
            if (day) {
                switch (day) {
                    case "numeric":
                        pattern.append("d")
                        break
                    case "2-digit":
                        pattern.append("dd")
                        break
                }
            }
        }

        private appendDayPeriodPattern(pattern: StringBuilder): void {
            const dayPeriod = this.options.dayPeriod
            if (dayPeriod) {
                pattern.append("B")
            }
        }

        private appendEraPattern(pattern: StringBuilder): void {
            const optEra = this.options.era
            if (optEra) {
                switch (optEra) {
                    case "long":
                        pattern.append("GGGG")
                        break
                    case "short":
                        pattern.append("GGG")
                        break
                    case "narrow":
                        pattern.append("GGGGG")
                        break
                }
            }
        }

        private getTimePattern(): string {
            const timePattern = new StringBuilder()

            this.appendHourPattern(timePattern)
            this.appendMinutePattern(timePattern)
            this.appendSecondPattern(timePattern)
            this.appendFractionalSecondPattern(timePattern)
            this.appendTimeZoneNamePattern(timePattern)
            
            return timePattern.toString()
        }

        private appendHourPattern(pattern: StringBuilder): void {
            const optHour = this.options.hour
            if (optHour) {
                const hourSymbol = this.getHourSymbol()
                switch (optHour) {
                    case "2-digit":
                        pattern.append(hourSymbol.repeat(2))
                        break
                    case "numeric":
                        pattern.append(hourSymbol)
                        break
                }
            }
        }

        private appendMinutePattern(pattern: StringBuilder): void {
            const optMinute = this.options.minute
            if (optMinute) {
                switch (optMinute) {
                    case "2-digit":
                        pattern.append("mm")
                        break
                    case "numeric":
                        pattern.append("m")
                        break
                }
            }
        }

        private appendSecondPattern(pattern: StringBuilder): void {
            const optSecond = this.options.second
            if (optSecond) {
                switch (optSecond) {
                    case "2-digit":
                        pattern.append("ss")
                        break
                    case "numeric":
                        pattern.append("s")
                        break
                }
            }
        }

        private appendFractionalSecondPattern(pattern: StringBuilder): void {
            const optFractionalSecond = this.options.fractionalSecondDigits
            if (optFractionalSecond) {
                if (optFractionalSecond < 1 || optFractionalSecond > 3) {
                    throw new RangeError("invalid digits value: " + optFractionalSecond)
                }
                pattern.append("S".repeat(optFractionalSecond))
            }
        }

        private appendTimeZoneNamePattern(pattern: StringBuilder): void {
            const optTimeZoneName = this.options.timeZoneName
            if (optTimeZoneName) {
                switch (optTimeZoneName) {
                    case "short":
                        pattern.append("O");
                        break
                    case "long":
                        pattern.append("zzzz")
                        break
                    case "shortGeneric":
                        pattern.append("VVVV")
                        break
                    case "shortOffset":
                        pattern.append("O")
                        break
                    case "longGeneric":
                        pattern.append("vvvv")
                        break
                    case "longOffset":
                        pattern.append("OOOO")
                        break
                }
            }
        }
    }

    type HourCycle = "h11" | "h12" | "h23" | "h24"
    type DateStyle = "full" | "long" | "medium" | "short"
    type TimeStyle = "full" | "long" | "medium" | "short"
    type TimeZoneName = "short" | "long" | "shortOffset" | "longOffset" | "shortGeneric" | "longGeneric"
    type DayPeriod = "narrow" | "short" | "long"
    type FormatMatcher = "basic" | "best fit"

    export class DateTimeFormatOptions {
        calendar?: string
        localeMatcher?: "lookup" | "best fit"
        dateStyle?: DateStyle
        day?: "numeric" | "2-digit"
        dayPeriod?: "narrow" | "short" | "long"
        era?: "long" | "short" | "narrow"
        formatMatcher?: "basic" | "best fit"
        // NOTE(cheezzario) 'number' literal types are not supported #23963
        // fractionalSecondDigits?: 1 | 2 | 3
        fractionalSecondDigits?: number
        hour12?: boolean
        hour?: "numeric" | "2-digit"
        hourCycle?: "h11" | "h12" | "h23" | "h24"
        minute?: "numeric" | "2-digit"
        month?: "numeric" | "2-digit" | "long" | "short" | "narrow"
        numberingSystem?: string
        second?: "numeric" | "2-digit"
        timeStyle?: TimeStyle
        timeZone?: string
        timeZoneName?: TimeZoneName
        weekday?: "long" | "short" | "narrow"
        year?: "numeric" | "2-digit"

        constructor() {}

        constructor(options: Intl.DateTimeFormatOptions) {
            this.calendar = options.calendar
            this.localeMatcher = options.localeMatcher
            this.dateStyle = options.dateStyle
            this.day = options.day
            this.dayPeriod = options.dayPeriod
            this.era = options.era
            this.formatMatcher = options.formatMatcher
            this.fractionalSecondDigits = options.fractionalSecondDigits
            this.hour12 = options.hour12
            this.hour = options.hour
            this.hourCycle = options.hourCycle
            this.minute = options.minute
            this.month = options.month
            this.numberingSystem = options.numberingSystem
            this.second = options.second
            this.timeStyle = options.timeStyle
            this.timeZone = options.timeZone
            this.timeZoneName = options.timeZoneName
            this.weekday = options.weekday
            this.year = options.year
        }

        static isEmpty(options: DateTimeFormatOptions): boolean {
            return options.localeMatcher === undefined 
                    && options.dateStyle === undefined 
                    && options.day === undefined 
                    && options.dayPeriod === undefined 
                    && options.era === undefined
                    && options.formatMatcher === undefined 
                    && options.fractionalSecondDigits === undefined 
                    && options.hour12 === undefined 
                    && options.hour === undefined 
                    && options.hourCycle === undefined 
                    && options.minute === undefined 
                    && options.month === undefined 
                    && options.numberingSystem === undefined 
                    && options.second === undefined 
                    && options.timeStyle === undefined 
                    && options.timeZone === undefined 
                    && options.timeZoneName === undefined 
                    && options.weekday === undefined 
                    && options.year === undefined
        }

        public toString(): string {
            return JSON.stringify(this)
        }
    }

    class DateTimeFormatPartImpl implements DateTimeFormatPart {
        private _type: DateTimeFormatPartTypes
        private _value: String

        constructor() {
            this._type = "literal"
            this._value = ""
        }

        constructor(type: DateTimeFormatPartTypes, value: string) {
            this._type = type
            this._value = value
        }

        get type(): DateTimeFormatPartTypes {
            return this._type
        }

        set type(value: DateTimeFormatPartTypes) {
            this._type = value
        }

        get value(): string {
            return this._value
        }

        set value(value: string) {
            this._value = value
        }
    }

    class DateTimeRangeFormatPartImpl extends DateTimeFormatPartImpl implements DateTimeRangeFormatPart {
        private  _source: DateTimeRangeFormatPartSource

        constructor() {
            this._source = "shared"
        }

        constructor(type: DateTimeFormatPartTypes, value: string, source: DateTimeRangeFormatPartSource) {
            super(type, value)
            this._source = source
        }

        get source(): DateTimeRangeFormatPartSource {
            return this._source
        }

        set source(source: DateTimeRangeFormatPartSource) {
            this._source = source
        }
    }

    class ResolvedDateTimeFormatOptionsImpl implements ResolvedDateTimeFormatOptions {
        private _calendar: string
        private _dateStyle?: DateStyle
        private _day?: string
        private _dayPeriod?: DayPeriod
        private _era?: string
        private _formatMatcher?: FormatMatcher
        private _fractionalSecondDigits?: number
        private _hour12?: boolean
        private _hour?: string
        private _hourCycle?: HourCycle
        private _locale: string
        private _minute?: string
        private _month?: string
        private _numberingSystem: string
        private _second?: string
        private _timeStyle?: TimeStyle
        private _timeZone: string
        private _timeZoneName?: string
        private _weekday?: string
        private _year?: string

        constructor(locale: string, calendar: string, numberingSystem: string, timeZone: string) {
            this._locale = locale
            this._calendar = calendar
            this._numberingSystem = numberingSystem
            this._timeZone = timeZone
        }

        get calendar(): string {
            return this._calendar
        }
        set calendar(calendar: string) {
            this._calendar = calendar
        }

        get dateStyle(): DateStyle | undefined {
            return this._dateStyle
        }
        set dateStyle(dateStyle?: DateStyle) {
            this._dateStyle = dateStyle
        }

        get day(): string | undefined {
            return this._day
        }
        set day(day?: string) {
            this._day = day
        }

        get dayPeriod(): DayPeriod | undefined {
            return this._dayPeriod
        }
        set dayPeriod(dayPeriod?: DayPeriod) {
            this._dayPeriod = dayPeriod
        }

        get era(): string | undefined {
            return this._era
        }
        set era(era?: string) {
            this._era = era
        }

        get formatMatcher(): FormatMatcher | undefined {
            return this._formatMatcher
        }
        set formatMatcher(formatMatcher?: FormatMatcher) {
            this._formatMatcher = formatMatcher
        }

        get fractionalSecondDigits(): number | undefined {
            return this._fractionalSecondDigits
        }
        set fractionalSecondDigits(fractionalSecondDigits?: number) {   
            this._fractionalSecondDigits = fractionalSecondDigits
        }

        get hour12(): boolean | undefined {
            return this._hour12
        }
        set hour12(hour12?: boolean) {
            this._hour12 = hour12
        }

        get hour(): string | undefined {
            return this._hour
        }
        set hour(hour?: string) {   
            this._hour = hour
        }

        get hourCycle(): HourCycle | undefined {
            return this._hourCycle
        }
        set hourCycle(hourCycle?: HourCycle) {
            this._hourCycle = hourCycle
        }

        get locale(): string {
            return this._locale
        }
        set locale(locale: string) {
            this._locale = locale
        }

        get minute(): string | undefined {
            return this._minute
        }
        set minute(minute?: string) {   
            this._minute = minute
        }

        get month(): string | undefined {
            return this._month
        }
        set month(month?: string) {   
            this._month = month
        }

        get numberingSystem(): string {
            return this._numberingSystem
        }
        set numberingSystem(numberingSystem: string) {   
            this._numberingSystem = numberingSystem
        }

        get second(): string | undefined {
            return this._second
        }
        set second(second?: string) {   
            this._second = second
        }

        get timeStyle(): TimeStyle | undefined {
            return this._timeStyle
        }
        set timeStyle(timeStyle?: TimeStyle) {   
            this._timeStyle = timeStyle
        }

        get timeZone(): string {
            return this._timeZone
        }
        set timeZone(timeZone: string) {   
            this._timeZone = timeZone
        }

        get timeZoneName(): string | undefined {
            return this._timeZoneName
        }
        set timeZoneName(timeZoneName?: string) {   
            this._timeZoneName = timeZoneName
        }

        get weekday(): string | undefined {
            return this._weekday
        }
        set weekday(weekday?: string) {   
            this._weekday = weekday
        }

        get year(): string | undefined {
            return this._year
        }
        set year(year?: string) {   
            this._year = year
        }
    }

} // namespace Intl
