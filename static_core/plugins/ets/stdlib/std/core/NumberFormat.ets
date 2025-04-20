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
            throw new RangeError("Invalid currency")
        }

        if ((options.unit != undefined) && !RefUnitsMap.has(options.unit!)) {
            throw new RangeError("Invalid unit")
        }

        if (options.style != undefined) {
            if ((options.style! == "unit") && (options.unit == undefined)) {
                throw new RangeError("Style is unit but unit is undefined")
            }
            if ((options.style! == "currency") && (options.currency == undefined)) {
                throw new RangeError("Style is currency but currency is undefined")
            }
        }

        if (options.maximumFractionDigits != undefined) {
            if (!(options.maximumFractionDigits! >= 0 && options.maximumFractionDigits! <= 100)) {
                throw new RangeError("Invalid maximumFractionDigits")
            }
            if (options.minimumFractionDigits != undefined) {
                if (options.maximumFractionDigits! < options.minimumFractionDigits!) {
                    throw new RangeError("Invalid maximumFractionDigits")
                }
            }
        }
        if ((options.minimumFractionDigits != undefined)
            && !(options.minimumFractionDigits! >= 0 && options.minimumFractionDigits! <= 100)) {
            throw new RangeError("Invalid minimumFractionDigits")
        }

        if (options.maximumSignificantDigits != undefined) {
            if (!(options.maximumSignificantDigits! >= 1 && options.maximumSignificantDigits! <= 21)) {
                throw new RangeError("Invalid maximumSignificantDigits")
            }
            if (options.minimumFractionDigits != undefined) {
                if (options.maximumSignificantDigits! < options.minimumSignificantDigits!) {
                    throw new RangeError("Invalid maximumSignificantDigits")
                }
            }
        }
        if ((options.minimumSignificantDigits != undefined) && !(options.minimumSignificantDigits! >= 1 && options.minimumSignificantDigits! <= 21)) {
            throw new RangeError("Invalid minimumSignificantDigits")
        }

        if ((options.minimumIntegerDigits != undefined) && !(options.minimumIntegerDigits! >= 1 && options.minimumIntegerDigits! <= 21)) {
            throw new RangeError("Invalid minimumIntegerDigits")
        }

        if (options.notation == "compact" && options.compactDisplay == undefined) {
            throw new RangeError("Notaion is compact but compactDisplay is undefined")
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
        public constructor(locales?: Intl.BCP47LanguageTag | Intl.BCP47LanguageTag[], options?: NumberFormatOptions) {
            // Set new options or default decimal style
            this.options = NumberFormat.assignOptionsDefault(options)

            if (locales == undefined) {
                if (options != undefined) {
                    throw new Error("Incorrect locale information provided")
                }
            } else {
                let loc: Array<string>
                if (locales! instanceof string) {
                    loc = Array.of<string>(locales! as string)
                } else {
                    loc = Array.from<string>(locales! as string[])
                }
                if (loc.includes("") && options != undefined) {
                    throw new Error("Incorrect locale information provided")
                }
            }

            let locMatcher = options?.localeMatcher ?? "best fit"
            this.options.locale = locMatcher == "lookup" ? intlLookUpLocale(locales ?? "") : intlBestFitLocale(locales ?? "")
            if (options != undefined) {
                let localOptions: Intl.NumberFormatOptions = options!; // Used to reduce amount of calls options!
                checkOptions(localOptions)
                if (localOptions.style != undefined) {
                    this.options.style = localOptions.style!

                    // Set Default fields for percent style
                    if (this.options.style == "percent") {
                        if (localOptions.maximumFractionDigits == undefined) {
                            this.options.maximumFractionDigits = 0
                        }
                        if (localOptions.minimumFractionDigits == undefined) {
                            this.options.minimumFractionDigits = 0
                        }
                        if (localOptions.useGrouping == undefined) {
                            this.options.useGrouping = true
                        }
                    }

                    // Set Default fields for currency style
                    if (this.options.style == "currency") {
                        if (localOptions.currencySign == undefined) {
                            this.options.currencySign = "standard";
                        }
                        if (localOptions.currencyDisplay == undefined) {
                            this.options.currencyDisplay = "symbol";
                        }
                        if (localOptions.minimumFractionDigits == undefined) {
                            this.options.minimumFractionDigits = 2;
                        }
                        if (localOptions.maximumFractionDigits ==  undefined) {
                            this.options.maximumFractionDigits = 2;
                        }
                        if (localOptions.useGrouping == undefined) {
                            this.options.useGrouping = true
                        }
                    }
                }
            }
            this.options.numberingSystem = "latn" // NOTE (ilyakorobkov): to define this field needs to fix issue #24827
            this.minFracStr = Number.toString(this.options.minimumFractionDigits)
            this.maxFracStr = Number.toString(this.options.maximumFractionDigits)
            this.minSignStr = this.options.minimumSignificantDigits != undefined ? Number.toString(this.options.minimumSignificantDigits!) : ""
            this.maxSignStr = this.options.maximumSignificantDigits != undefined ? Number.toString(this.options.maximumSignificantDigits!) : ""
            this.minIntStr = Number.toString(this.options.minimumIntegerDigits)
            if (this.options.useGrouping) {
                this.useGroupingStr = "true"
            } else {
                this.useGroupingStr = "false"
                if (options == undefined) {
                    this.useGroupingStr = ""
                } else {
                    if (options!.useGrouping == undefined) {
                        this.useGroupingStr = ""
                    }
                }
            }
        }

        private static assignOptionsDefault(options?: NumberFormatOptions) {
            let newOptions = new ResolvedNumberFormatOptions
            // Set new options or default decimal style
            newOptions.compactDisplay = options?.compactDisplay ?? undefined
            newOptions.currencySign = options?.currencySign ?? undefined
            newOptions.currency = options?.currency ?? undefined
            newOptions.currencyDisplay = options?.currencyDisplay ?? undefined
            newOptions.locale = intlBestFitLocale(options?.localeMatcher ?? "")
            newOptions.maximumFractionDigits = options?.maximumFractionDigits ?? 3
            newOptions.minimumFractionDigits = options?.minimumFractionDigits ?? 0
            newOptions.minimumIntegerDigits = options?.minimumIntegerDigits ?? 1
            newOptions.maximumSignificantDigits = options?.maximumSignificantDigits ?? undefined
            newOptions.minimumSignificantDigits = options?.minimumSignificantDigits ?? undefined
            newOptions.notation = options?.notation ?? "standard"
            newOptions.numberingSystem = "latn" // NOTE (ilyakorobkov): to define this field needs to fix issue #24827
            newOptions.signDisplay = options?.signDisplay ?? "auto"
            newOptions.style = options?.style ?? "decimal"
            newOptions.unit = options?.unit ?? undefined
            newOptions.unitDisplay = options?.unitDisplay ?? undefined
            newOptions.useGrouping = options?.useGrouping ?? false
            return newOptions
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
            return this.formatDouble(value);
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
                return this.formatDecStr(value.toString());
            }
            return this.formatDouble(value);
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
                return this.formatRangeDoubleDouble(start as double, end as double);
            } else if (!isStartBigInt && isEndBigInt) {
                return this.formatRangeDoubleDecStr(start as double, (end as BigInt).toString());
            } else if (isStartBigInt && !isEndBigInt) {
                return this.formatRangeDecStrDouble((start as BigInt).toString(), end as double);
            }
            return this.formatRangeDecStrDecStr((start as BigInt).toString(), (end as BigInt).toString());
        }

        /**
        * Returns an Array of objects representing the range of numbers in parts that can be used for custom locale-aware formatting.
        *
        * @param start Start range to format a number or BigInt
        * @param end End range to format a number or BigInt
        *
        * @throws TypeError if start or end is NaN
        *
        * @returns An Array of objects containing the formatted range in parts
        */
        public formatRangeToParts(start: number | bigint, end: number | bigint): NumberRangeFormatPart[] {
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
                return this.formatToRangePartsDoubleDouble(start as double, end as double);
            } else if (!isStartBigInt && isEndBigInt) {
                return this.formatToRangePartsDoubleDecStr(start as double, (end as BigInt).toString());
            } else if (isStartBigInt && !isEndBigInt) {
                return this.formatToRangePartsDecStrDouble((start as BigInt).toString(), end as double);
            }
            return this.formatToRangePartsDecStrDecStr((start as BigInt).toString(), (end as BigInt).toString());
        }

        /**
        * Returns an Array of objects representing the number string in parts that can be used for custom locale-aware formatting.
        *
        * @param value A number or BigInt to format
        *
        * @returns An Array of objects containing the formatted number in parts
        */
        public formatToParts(value: number | bigint): NumberFormatPart[] {
            if (value instanceof BigInt) {
                return this.formatToPartsDecStr(value.toString());
            }
            return this.formatToPartsDouble(value);
        }

        /**
        *
        * Returns resolved options
        *
        * @return resolved options
        */
        public resolvedOptions(): ResolvedNumberFormatOptions {
            return this.options
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
            if (options != undefined) {
                checkOptions(options!)
            }
            let locMatcher = options?.localeMatcher ?? "best fit"
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
        private readonly minFracStr: string;
        private readonly maxFracStr: string;
        private readonly minSignStr: string;
        private readonly maxSignStr: string;
        private readonly minIntStr: string;
        private readonly useGroupingStr: string;

        private native formatDouble(value: double): string;
        private native formatDecStr(value: string): string;
        private native formatRangeDoubleDouble(start: number, end: number): string;
        private native formatRangeDoubleDecStr(start: number, end: string): string;
        private native formatRangeDecStrDouble(start: string, end: number): string;
        private native formatRangeDecStrDecStr(start: string, end: string): string;
        private native formatToPartsDouble(value: double): NumberFormatPart[];
        private native formatToPartsDecStr(value: string): NumberFormatPart[];
        private native formatToRangePartsDoubleDouble(start: number, end: number): NumberRangeFormatPart[];
        private native formatToRangePartsDoubleDecStr(start: number, end: string): NumberRangeFormatPart[];
        private native formatToRangePartsDecStrDouble(start: string, end: number): NumberRangeFormatPart[];
        private native formatToRangePartsDecStrDecStr(start: string, end: string): NumberRangeFormatPart[];
    }

    export class NumberRangeFormatPart {
        type: NumberFormatPartTypes;
        value: string;
        source: "startRange" | "endRange" | "shared"

        public constructor(
            type: NumberFormatPartTypes,
            value: string,
            source: "startRange" | "endRange" | "shared")
        {
            this.type = type;
            this.value = value;
            this.source = source;
        }

    }

    export class NumberFormatPart {
        // No default value in TS/JS, because of this class
        //  is only used as returned result for
        //  function Intl.NumberFormat.formatToParts(..)
        type: NumberFormatPartTypes;
        value: string;

        public constructor(type: NumberFormatPartTypes, value: string)
        {
            this.type = type;
            this.value = value;
        }
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

    // NOTE(ilyakorobkov) replace class with interface after fix issue #22050, #24106
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
