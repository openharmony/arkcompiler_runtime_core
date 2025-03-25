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

        private readonly locale: string | undefined
        private readonly options: DateTimeFormatOptions
        private readonly pattern: string

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

        public constructor(locales?: string | string[], options?: DateTimeFormatOptions) {
            if (locales !== undefined) {
                const lookupLocale = options != null && options!.localeMatcher == "lookup"
                this.locale = lookupLocale ? intlLookUpLocale(locales) : intlBestFitLocale(locales)
            } else {
                this.locale = undefined
            }

            if (options === undefined || DateTimeFormatOptions.isEmpty(options)) {
                this.options = DateTimeFormat.DEFAULT_OPTIONS
            } else {
                this.options = this.getResolvedOptions(options!)
            }

            this.pattern = this.getPattern()
        }

        public format(date?: Date | number): string {
            if (date instanceof Date) {
                return this.formatImpl(date.getTime())
            } else {
                return this.formatImpl(date!)
            }
        }

        private getPattern(): string {
            const datePattern = this.getDatePattern()
            const timePattern = this.getTimePattern()

            if (datePattern.isEmpty()) {
                return timePattern
            } else {
                if (timePattern.isEmpty()) {
                    return datePattern
                } else {
                    return datePattern + timePattern
                }
            }
        }

        private getDatePattern(): string {
            const datePattern = new StringBuilder()

            this.appendYearPattern(datePattern)
            this.appendMonthPattern(datePattern)
            this.appendWeekdayPattern(datePattern)
            this.appendDayPattern(datePattern)
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

        private native formatImpl(timestamp: number): string

        private native getLocaleHourCycle(): HourCycle| undefined
    }

    type HourCycle = "h11" | "h12" | "h23" | "h24"
    type DateStyle = "full" | "long" | "medium" | "short"
    type TimeStyle = "full" | "long" | "medium" | "short"
    type TimeZoneName = "short" | "long" | "shortOffset" | "longOffset" | "shortGeneric" | "longGeneric"

    export class DateTimeFormatOptions {
        localeMatcher?: "lookup" | "best fit"
        dateStyle?: DateStyle
        day?: "numeric" | "2-digit"
        dayPeriod?: "narrow" | "short" | "long"
        era?: "long" | "short" | "narrow"
        formatMatcher?: "basic" | "best fit"
        // NOTE(cheezzario) 'number' literal types are not supported #23963
        // fractionalSecondsDigits?: 1 | 2 | 3
        fractionalSecondsDigits?: number
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

        static isEmpty(options: DateTimeFormatOptions): boolean {
            return options.localeMatcher === undefined 
                    && options.dateStyle === undefined 
                    && options.day === undefined 
                    && options.dayPeriod === undefined 
                    && options.era === undefined
                    && options.formatMatcher === undefined 
                    && options.fractionalSecondsDigits === undefined 
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
    }
}
