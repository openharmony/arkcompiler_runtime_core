/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

    const Currencies: ReadonlyArray<string> = Array.of<string>("AED", "AFN", "ALL", "AMD", "ANG", "AOA", "ARS", "AUD",
        "AWG", "AZN", "BAM", "BBD", "BDT", "BGN", "BHD", "BIF", "BMD", "BND", "BOB", "BRL", "BSD", "BTN", "BWP", "BYN",
        "BZD", "CAD", "CDF", "CHF", "CLP", "CNY", "COP", "CRC", "CUC", "CUP", "CVE", "CZK", "DJF", "DKK", "DOP", "DZD",
        "EGP", "ERN", "ETB", "EUR", "FJD", "FKP", "GBP", "GEL", "GHS", "GIP", "GMD", "GNF", "GTQ", "GYD", "HKD", "HNL",
        "HRK", "HTG", "HUF", "IDR", "ILS", "INR", "IQD", "IRR", "ISK", "JMD", "JOD", "JPY", "KES", "KGS", "KHR", "KMF",
        "KPW", "KRW", "KWD", "KYD", "KZT", "LAK", "LBP", "LKR", "LRD", "LSL", "LYD", "MAD", "MDL", "MGA", "MKD", "MMK",
        "MNT", "MOP", "MRU", "MUR", "MVR", "MWK", "MXN", "MYR", "MZN", "NAD", "NGN", "NIO", "NOK", "NPR", "NZD", "OMR",
        "PAB", "PEN", "PGK", "PHP", "PKR", "PLN", "PYG", "QAR", "RON", "RSD", "RUB", "RWF", "SAR", "SBD", "SCR", "SDG",
        "SEK", "SGD", "SHP", "SLL", "SOS", "SRD", "SSP", "STN", "SVC", "SYP", "SZL", "THB", "TJS", "TMT", "TND", "TOP",
        "TRY", "TTD", "TWD", "TZS", "UAH", "UGX", "USD", "UYU", "UZS", "VES", "VND", "VUV", "WST", "XAF", "XCD", "XDR",
        "XOF", "XPF", "XSU", "YER", "ZAR", "ZMW", "ZWL")
    const RefCurrenciesMap = new Map<string, number>()
    Currencies.forEach((value, index) => { RefCurrenciesMap.set(value, index) })
    const Units: ReadonlyArray<string> = Array.of<string>("acre", "bit", "byte", "celsius", "centimeter", "day", "degree",
        "fahrenheit", "fluid-ounce", "foot", "gallon", "gigabit", "gigabyte", "gram", "hectare", "hour", "inch", "kilobit",
        "kilobyte", "kilogram", "kilometer", "liter", "megabit", "megabyte", "meter", "microsecond", "mile", "mile-scandinavian",
        "milliliter", "millimeter", "millisecond", "minute", "month", "nanosecond", "ounce", "percent", "petabyte", "pound",
        "second", "stone", "terabit", "terabyte", "week", "yard", "year")
    const RefUnitsMap = new Map<string, number>()
    Units.forEach((value, index) => { RefUnitsMap.set(value, index) })

    function checkOptions(options: NumberFormatOptions) {
        if ((options.currency != undefined) && !RefCurrenciesMap.has(options.currency!.toUpperCase())) {
            throw new RangeError("currency value is out of range")
        }

        if ((options.unit != undefined) && !RefUnitsMap.has(options.unit!)) {
            throw new RangeError("unit value is out of range")
        }

        if (options.style != undefined) {
            if ((options.style! == "unit") && (options.unit == undefined)) {
                throw new RangeError("Unit code is required with unit style")
            }
            if ((options.style! == "currency") && (options.currency == undefined)) {
                throw new RangeError("Currency code is required with currency style")
            }
        }

        if (options.maximumFractionDigits != undefined) {
            if (!(options.maximumFractionDigits! >= 0 && options.maximumFractionDigits! <= 100)) {
                throw new RangeError("maximumFractionDigits value is out of range")
            }
            if (options.minimumFractionDigits != undefined) {
                if (options.maximumFractionDigits! < options.minimumFractionDigits!) {
                    throw new RangeError("maximumFractionDigits value is out of range")
                }
            }
        }
        if ((options.minimumFractionDigits != undefined)
            && !(options.minimumFractionDigits! >= 0 && options.minimumFractionDigits! <= 100)) {
            throw new RangeError("minimumFractionDigits value is out of range")
        }

        if (options.maximumSignificantDigits != undefined) {
            if (!(options.maximumSignificantDigits! >= 1 && options.maximumSignificantDigits! <= 21)) {
                throw new RangeError("maximumSignificantDigits value is out of range")
            }
            if (options.minimumFractionDigits != undefined) {
                if (options.maximumSignificantDigits! < options.minimumSignificantDigits!) {
                    throw new RangeError("maximumSignificantDigits value is out of range")
                }
            }
        }
        if ((options.minimumSignificantDigits != undefined)
            && !(options.minimumSignificantDigits! >= 1 && options.minimumSignificantDigits! <= 21)) {
            throw new RangeError("minimumSignificantDigits value is out of range")
        }

        if ((options.minimumIntegerDigits != undefined)
            && !(options.minimumIntegerDigits! >= 1 && options.minimumIntegerDigits! <= 21)) {
            throw new RangeError("minimumIntegerDigits value is out of range")
        }
    }

    export class NumberFormat {

        /**
        * The Intl.NumberFormat() constructor creates Intl.NumberFormat objects.
        *
        * @param locales locale(s) from which to load formats and symbols for number formatting
        *
        * @param options options
        */
        public constructor(locales?: string | string[], options?: NumberFormatOptions) {
            // Set default fields for decimal style
            this.options = { locale: intlBestFitLocale(""),
                numberingSystem: "latn",  // NOTE (ilyakorobkov): to define this field call Intl.supportedValuesOf() when it will be added
                style: "decimal",
                minimumIntegerDigits: 1,
                maximumFractionDigits: 3,
                minimumFractionDigits: 0,
                useGrouping: false,
                notation: "standard",
                signDisplay: "auto"
            } as ResolvedNumberFormatOptions

            let locMatcher = "best fit"
            if (options != undefined) {
                if (options.localeMatcher != undefined) {
                    locMatcher = options.localeMatcher!
                }
            }
            let loc: Array<string>
            if (locales == undefined) {
                if (options != undefined) {
                    throw new Error("Incorrect locale information provided")
                }
            } else {
                if (locales! instanceof string)
                    loc = Array.of<string>(locales! as string)
                else {
                    loc = Array.from<string>(locales! as string[])
                }
                if (loc.includes("") && options != undefined) {
                    throw new Error("Incorrect locale information provided")
                }
            }

            this.options.locale = locMatcher == "lookup" ? intlLookUpLocale(locales ?? "") : intlBestFitLocale(locales ?? "")

            if (options != undefined) {
                checkOptions(options!)
                this.options.compactDisplay = options!.compactDisplay
                this.options.currency = options!.currency
                this.options.currencyDisplay = options!.currencyDisplay
                this.options.currencySign = options!.currencySign
                if (options!.maximumFractionDigits != undefined) {
                    this.options.maximumFractionDigits = options!.maximumFractionDigits!
                }
                this.options.maximumSignificantDigits = options!.maximumSignificantDigits
                if (options!.minimumFractionDigits != undefined) {
                    this.options.minimumFractionDigits = options!.minimumFractionDigits!
                }
                if (options!.minimumIntegerDigits != undefined) {
                    this.options.minimumIntegerDigits = options!.minimumIntegerDigits!
                }
                this.options.minimumSignificantDigits = options!.minimumSignificantDigits
                if (options!.notation != undefined) {
                    this.options.notation = options!.notation!
                }
                if (options!.signDisplay != undefined) {
                    this.options.signDisplay = options!.signDisplay!
                }
                if (options!.useGrouping != undefined) {
                    this.options.useGrouping = options!.useGrouping!
                }
                this.options.unit = options!.unit
                this.options.unitDisplay = options!.unitDisplay

                if (options!.style != undefined) {
                    this.options.style = options!.style!

                    // Set Default fields for percent style
                    if (this.options.style == "percent") {
                        if (options!.maximumFractionDigits == undefined) {
                            this.options.maximumFractionDigits = 0
                        }
                        if (options!.minimumFractionDigits == undefined) {
                            this.options.minimumFractionDigits = 0
                        }
                        if (options!.useGrouping == undefined) {
                            this.options.useGrouping = true
                        }
                        this.options.numberingSystem = "latn" // NOTE (ilyakorobkov): to define this field call Intl.supportedValuesOf() when it will be added
                    }

                    // Set Default fields for currency style
                    if (this.options.style == "currency") {
                        if (options!.currencySign == undefined) {
                            this.options.currencySign = "standard";
                        }
                        if (options!.currencyDisplay == undefined) {
                            this.options.currencyDisplay = "symbol";
                        }
                        if (options!.minimumFractionDigits == undefined) {
                            this.options.minimumFractionDigits = 2;
                        }
                        if (options!.maximumFractionDigits ==  undefined) {
                            this.options.maximumFractionDigits = 2;
                        }
                        if (options!.useGrouping == undefined) {
                            this.options.useGrouping = true
                        }
                        this.options.numberingSystem = "latn" // NOTE (ilyakorobkov): to define this field call Intl.supportedValuesOf() when it will be added
                    }
                }
            }
        }

        /**
        * Creates a new instance of an Intl.NumberFormat with the specified locale and options
        *
        * @param locales locale(s) from which to load formats and symbols for number formatting
        *
        * @param options options
        *
        * @returns A new Array instance with the specified length
        */
        public static $_invoke(locales?: string | string[], options?: NumberFormatOptions): NumberFormat {
            return new NumberFormat(locales, options)
        }

        /**
        * Formats a number according to the locale and formatting options of this Intl.NumberFormat object.
        *
        * @param value A number to format.
        *
        * @returns string representing the given number formatted according to the locale and formatting options of this Intl.NumberFormat object.
        */
        public format(value: number): string {
            return NumberFormat.formatDouble(this.options.locale, value);
        }

        /**
        * Formats a number according to the locale and formatting options of this Intl.NumberFormat object.
        *
        * @param value A number or BigInt to format
        *
        * @returns A string representing the given number formatted according to the locale and formatting options of this Intl.NumberFormat object.
        */
        public format(value: number | bigint): string {
            if (value instanceof BigInt) {
                return NumberFormat.formatDecStr(this.options.locale, value.toString());
            }
            return NumberFormat.formatDouble(this.options.locale, value);
        }

        /**
        * Formats a range of numbers according to the locale and formatting options of this Intl.NumberFormat object.
        *
        * @param start Start range to format a number or BigInt
        *
        * @param end End range to format a number or BigInt
        *
        * @throws TypeError if start or end is NaN
        *
        * @returns A string representing the given range of numbers formatted according to the locale and formatting options of this Intl.NumberFormat object.
        */
        public formatRange(start: number | bigint, end: number | bigint): string {
            let isStartBigInt: boolean = false;
            if (start instanceof BigInt) {
                isStartBigInt = true;
            } else if (isNaN(start as number)) {
                throw new TypeError("Invalid start: NaN");
            }
            let isEndBigInt: boolean = false;
            if (end instanceof BigInt) {
                isEndBigInt = true;
            } else if (isNaN(end as number)) {
                throw new TypeError("Invalid end: NaN");
            }

            if (!isStartBigInt && !isEndBigInt) {
                return NumberFormat.formatRangeDoubleDouble(this.options.locale, start as double, end as double);
            } else if (!isStartBigInt && isEndBigInt) {
                return NumberFormat.formatRangeDoubleDecStr(this.options.locale, start as double, (end as BigInt).toString());
            } else if (isStartBigInt && !isEndBigInt) {
                return NumberFormat.formatRangeDecStrDouble(this.options.locale, (start as BigInt).toString(), end as double);
            }
            return NumberFormat.formatRangeDecStrDecStr(this.options.locale, (start as BigInt).toString(), (end as BigInt).toString());
        }

        /**
        *
        * Returns resolved options
        *
        * @return resolved options
        */
        public resolvedOptions(): ResolvedNumberFormatOptions {
            return this.options;
        }

        /**
        * Returns supported locales
        *
        * @param locales locales
        *
        * @param options options
        *
        * @return supported locales
        */
        static supportedLocalesOf(locales: string | string[], options?: NumberFormatOptions): string[] {
            let locMatcher = "best fit"
            if (options != undefined) {
                if (options.localeMatcher != undefined) {
                    locMatcher = options.localeMatcher!
                }
            }
            let loc: Array<string>
            if (locales! instanceof string)
                loc = Array.of<string>(locales! as string)
            else {
                loc = Array.from<string>(locales! as string[])
            }
            if (loc.includes("")) {
                throw new Error("Incorrect locale information provided")
            }

            let result: string[]
            if (locales instanceof string) {
                result = [locales]
            } else {
                result = locales
            }
            for (let i = 0; i < result.length; ++i) {
                result[i] = locMatcher == "lookup" ? intlLookUpLocale(result[i]) : intlBestFitLocale(result[i])
            }
            return result
        }

        private readonly options: ResolvedNumberFormatOptions

        private static native formatDouble(locale: string, value: double): string;
        private static native formatDecStr(locale: string, value: string): string;
        private static native formatRangeDoubleDouble(locale: string, start: number, end: number): string;
        private static native formatRangeDoubleDecStr(locale: string, start: number, end: string): string;
        private static native formatRangeDecStrDouble(locale: string, start: string, end: number): string;
        private static native formatRangeDecStrDecStr(locale: string, start: string, end: string): string;
    }

    export class NumberRangeFormatPart {
        source: "startRange" | "endRange" | "shared"
    }

    export class NumberFormatPart {
        type: NumberFormatPartTypes; // No default value in TS/JS, because of this class is only used as returned result for function Intl.NumberFormat.formatToParts(..)
        value: string;

        public constructor() {}
    }

    // NOTE(ilyakorobkov) replace class with interface after fix issue #22050, #24106
    export class NumberFormatOptions {
        compactDisplay?: "short" | "long";
        currency?: string;
        currencyDisplay?: "code" | "symbol" | "narrowSymbol" | "name"
        currencySign?: "standard" | "accounting"
        localeMatcher?: "lookup" | "best fit"
        maximumFractionDigits?: number
        maximumSignificantDigits?: number
        minimumFractionDigits?: number
        minimumIntegerDigits?: number
        minimumSignificantDigits?: number
        notation?: "standard" | "scientific" | "engineering" | "compact"
        signDisplay?: "auto" | "never" | "always" | "exceptZero"
        style?: "decimal" | "percent" | "currency" | "unit"
        unit?: string
        unitDisplay?: "short" | "long" | "narrow"
        useGrouping?: boolean;
    }

    // NOTE(ilyakorobkov) replace class with interface after fix issues #22050, #24106
    export class ResolvedNumberFormatOptions {
        compactDisplay?: "short" | "long"
        currencySign?: "standard" | "accounting"
        currency?: string
        currencyDisplay?: "code" | "symbol" | "narrowSymbol" | "name"
        locale: Intl.BCP47LanguageTag;
        maximumFractionDigits: number
        maximumSignificantDigits?: number
        minimumFractionDigits: number
        minimumIntegerDigits: number
        minimumSignificantDigits?: number
        notation?: "standard" | "scientific" | "engineering" | "compact"
        numberingSystem: string
        signDisplay?: "auto" | "never" | "always" | "exceptZero"
        style: "decimal" | "percent" | "currency" | "unit"
        unit?: string
        unitDisplay?: "short" | "long" | "narrow"
        useGrouping: boolean
    }
}
