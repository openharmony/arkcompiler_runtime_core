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
                const localOptions: Intl.NumberFormatOptions = options!; // It's used to reduce amount of calls options!
                NumberFormat.checkOptions(localOptions)
                if (localOptions.style != undefined) {
                    this.options.style = localOptions.style!

                    // Handle percent style
                    if (this.options.style == "percent") {
                        if (localOptions.maximumFractionDigits == undefined) {
                            this.options.maximumFractionDigits = 0
                        }
                        if (localOptions.minimumFractionDigits == undefined) {
                            this.options.minimumFractionDigits = 0
                        }
                    }

                    // Handle currency style
                    if (this.options.style == "currency") {
                        if (localOptions.currencySign == undefined) {
                            this.options.currencySign = "standard";
                        }
                        if (localOptions.currencyDisplay == undefined) {
                            this.options.currencyDisplay = "symbol";
                        }
                        if (localOptions.minimumFractionDigits == undefined) {
                           this.options.minimumFractionDigits = this.currencyDigits(localOptions.currency);
                        }
                        if (localOptions.maximumFractionDigits == undefined) {
                            this.options.maximumFractionDigits = this.currencyDigits(localOptions.currency);
                        }
                    }

                    // Handle unit style
                    if (this.options.style == "unit") {
                        if (localOptions.unitDisplay == undefined) {
                            this.options.unitDisplay = "short";
                        }
                    }
                }

                // Handle notation compact
                // roundingPriority is "auto" by default
                const minFrac = localOptions.minimumFractionDigits
                const maxFrac = localOptions.maximumFractionDigits
                const minSign = localOptions.minimumSignificantDigits
                const maxSign = localOptions.maximumSignificantDigits
                if (this.options.isCompact) {
                    if (minFrac == undefined && maxFrac == undefined && minSign == undefined && maxSign == undefined) {
                        this.options.minimumFractionDigits = 0
                        this.options.maximumFractionDigits = 0
                        this.options.minimumSignificantDigits = 1
                        this.options.maximumSignificantDigits = 2
                    } else if (minFrac != undefined || maxFrac != undefined) {
                        // FractionDigits options apply, other ignored
                        this.options.minimumSignificantDigits = undefined
                        this.options.maximumSignificantDigits = undefined
                    }
                } else {
                    if (minSign != undefined || maxSign != undefined) {
                        // SignificantDigits options apply, other ignored
                        this.options.minimumFractionDigits = 0
                        this.options.maximumFractionDigits = 0
                    } else {
                        // FractionDigits options apply, other ignored
                        this.options.minimumSignificantDigits = undefined
                        this.options.maximumSignificantDigits = undefined
                    }
                }

                // Additional handle useGrouping (begin)
                if (localOptions.useGrouping == undefined) {
                    this.options.useGroupingStr = this.options.isCompact ? "min2" : ""
                } else if (this.options.useGrouping) {
                    this.options.useGroupingStr = "true"
                } else if (!this.options.useGrouping) {
                    this.options.useGroupingStr = "false"
                }
            } else {
                // Additional handle useGrouping (end)
                this.options.useGroupingStr = ""
            }

            // Handle numberingSystem
            this.options.numberingSystem = NumberFormat.getNumberingSystem(this.options.locale)

            this.options.minFracStr = Number.toString(this.options.minimumFractionDigits)
            this.options.maxFracStr = Number.toString(this.options.maximumFractionDigits)
            this.options.minSignStr = this.options.minimumSignificantDigits != undefined ? Number.toString(this.options.minimumSignificantDigits!) : ""
            this.options.maxSignStr = this.options.maximumSignificantDigits != undefined ? Number.toString(this.options.maximumSignificantDigits!) : ""
            this.options.minIntStr = Number.toString(this.options.minimumIntegerDigits)
        }

        private static checkOptions(options: NumberFormatOptions) {
            if (options.currency != undefined) {
                if (!RefCurrenciesMap.has(options.currency!.toUpperCase())) {
                    throw new RangeError("Invalid currency")
                }
            }

            if (options.unit != undefined) {
                if (!NumberFormat.isUnitCorrect(options.unit!)) {
                    throw new RangeError("Invalid unit")
                }
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

        /**
        * Assign new options or default decimal style and values
        *
        * @param options options
        *
        * @returns boolean true means notation is "compact". Otherwise, false
        */
        private static assignOptionsDefault(options?: NumberFormatOptions): ResolvedNumberFormatOptionsImpl {
            let newOptions = new ResolvedNumberFormatOptionsImpl
            // Set new options or default decimal style and values
            newOptions._currencySign = options?.currencySign ?? undefined
            newOptions._currency = options?.currency ?? undefined
            newOptions._currencyDisplay = options?.currencyDisplay ?? undefined
            newOptions._locale = Locale.defaultTag()
            newOptions._maximumFractionDigits = options?.maximumFractionDigits ?? 3
            newOptions._minimumFractionDigits = options?.minimumFractionDigits ?? 0
            if (newOptions._maximumFractionDigits < newOptions._minimumFractionDigits) {
                newOptions._maximumFractionDigits = newOptions._minimumFractionDigits
            }
            newOptions._minimumIntegerDigits = options?.minimumIntegerDigits ?? 1
            newOptions._maximumSignificantDigits = options?.maximumSignificantDigits ?? undefined
            newOptions._minimumSignificantDigits = options?.minimumSignificantDigits ?? undefined
            if (newOptions._minimumSignificantDigits != undefined) {
                if (newOptions._maximumSignificantDigits == undefined) {
                    newOptions._maximumSignificantDigits = 21
                }
            }
            if (newOptions._maximumSignificantDigits != undefined) {
                if (newOptions._minimumSignificantDigits == undefined) {
                    newOptions._minimumSignificantDigits = 1
                }
            }
            newOptions._notation = options?.notation ?? "standard"
            if (newOptions._notation == "compact") {
                newOptions._compactDisplay = options?.compactDisplay ?? "short"
                newOptions.isCompact = true
            } else {
                newOptions.isCompact = false
            }
            newOptions._numberingSystem = "latn"
            newOptions._signDisplay = options?.signDisplay ?? "auto"
            newOptions._style = options?.style ?? "decimal"
            newOptions._unit = options?.unit ?? undefined
            newOptions._unitDisplay = options?.unitDisplay ?? undefined
            newOptions._useGrouping = (options == undefined) ? true : (options!.useGrouping ?? true)
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
            return this.options.resolvedNumberFormatOptions()
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
                NumberFormat.checkOptions(options!)
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
            return locMatcher == "best fit" ? intlBestFitLocales(loc) : intlLookUpLocales(loc)
        }

        private readonly options: ResolvedNumberFormatOptionsImpl

        private native static isUnitCorrect(unit: string): boolean;
        private native static getNumberingSystem(locale: string): string;
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
        private native currencyDigits(currency: string | undefined): number;
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

    export interface NumberFormatOptions {
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

    export interface ResolvedNumberFormatOptions {
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

    class ResolvedNumberFormatOptionsImpl implements ResolvedNumberFormatOptions {
        public minFracStr: string;
        public maxFracStr: string;
        public minSignStr: string;
        public maxSignStr: string;
        public minIntStr: string;
        public useGroupingStr: string;
        public isCompact: boolean;

        private _compactDisplay?: "short" | "long";
        private _currencySign?: "standard" | "accounting";
        private _currency?: string;
        private _currencyDisplay?: "code" | "symbol" | "narrowSymbol" | "name";
        private _locale: Intl.BCP47LanguageTag;
        private _maximumFractionDigits: number;
        private _maximumSignificantDigits?: number;
        private _minimumFractionDigits: number;
        private _minimumIntegerDigits: number;
        private _minimumSignificantDigits?: number;
        private _notation?: "standard" | "scientific" | "engineering" | "compact";
        private _numberingSystem: string;
        private _signDisplay?: "auto" | "never" | "always" | "exceptZero";
        private _style: "decimal" | "percent" | "currency" | "unit";
        private _unit?: string;
        private _unitDisplay?: "short" | "long" | "narrow";
        private _useGrouping: boolean;

        set compactDisplay(value: "short" | "long" | undefined) {
            this._compactDisplay = value;
        }

        get compactDisplay(): "short" | "long" | undefined {
            return this._compactDisplay;
        }

        set currencySign(value: "standard" | "accounting" | undefined) {
            this._currencySign = value;
        }

        get currencySign(): "standard" | "accounting" | undefined {
            return this._currencySign;
        }

        set currency(value: string | undefined) {
            this._currency = value;
        }

        get currency(): string | undefined {
            return this._currency;
        }

        set currencyDisplay(value: "code" | "symbol" | "narrowSymbol" | "name" | undefined) {
            this._currencyDisplay = value;
        }

        get currencyDisplay(): "code" | "symbol" | "narrowSymbol" | "name" | undefined {
            return this._currencyDisplay;
        }

        set locale(value: Intl.BCP47LanguageTag) {
            this._locale = value;
        }

        get locale(): Intl.BCP47LanguageTag {
            return this._locale;
        }

        set maximumFractionDigits(value: number) {
            this._maximumFractionDigits = value;
        }

        get maximumFractionDigits(): number {
            return this._maximumFractionDigits;
        }

        set maximumSignificantDigits(value: number | undefined) {
            this._maximumSignificantDigits = value;
        }

        get maximumSignificantDigits(): number | undefined {
            return this._maximumSignificantDigits;
        }

        set minimumFractionDigits(value: number) {
            this._minimumFractionDigits = value;
        }

        get minimumFractionDigits(): number {
            return this._minimumFractionDigits;
        }

        set minimumIntegerDigits(value: number) {
            this._minimumIntegerDigits = value;
        }

        get minimumIntegerDigits(): number {
            return this._minimumIntegerDigits;
        }

        set minimumSignificantDigits(value: number | undefined) {
            this._minimumSignificantDigits = value;
        }

        get minimumSignificantDigits(): number | undefined {
            return this._minimumSignificantDigits;
        }

        set notation(value: "standard" | "scientific" | "engineering" | "compact" | undefined) {
            this._notation = value;
        }

        get notation(): "standard" | "scientific" | "engineering" | "compact" | undefined {
            return this._notation;
        }

        set numberingSystem(value: string) {
            this._numberingSystem = value;
        }

        get numberingSystem(): string {
            return this._numberingSystem;
        }

        set signDisplay(value: "auto" | "never" | "always" | "exceptZero" | undefined) {
            this._signDisplay = value;
        }

        get signDisplay(): "auto" | "never" | "always" | "exceptZero" | undefined {
            return this._signDisplay;
        }

        set style(value: "decimal" | "percent" | "currency" | "unit") {
            this._style = value;
        }

        get style(): "decimal" | "percent" | "currency" | "unit" {
            return this._style;
        }

        set unit(value: string | undefined) {
            this._unit = value;
        }

        get unit(): string | undefined {
            return this._unit;
        }

        set unitDisplay(value: "short" | "long" | "narrow" | undefined) {
            this._unitDisplay = value;
        }

        get unitDisplay(): "short" | "long" | "narrow" | undefined {
            return this._unitDisplay;
        }

        set useGrouping(value: boolean) {
            this._useGrouping = value;
        }

        get useGrouping(): boolean {
            return this._useGrouping;
        }

        public toString(): string {
            return JSON.stringify(this)
        }

        public resolvedNumberFormatOptions(): ResolvedNumberFormatOptions {
            let newOptions: ResolvedNumberFormatOptions = {}
            newOptions.compactDisplay = this._compactDisplay
            newOptions.currencySign = this._currencySign
            newOptions.currency = this._currency
            newOptions.currencyDisplay = this._currencyDisplay
            newOptions.locale = this._locale
            newOptions.maximumFractionDigits = this._maximumFractionDigits
            newOptions.maximumSignificantDigits = this._maximumSignificantDigits
            newOptions.minimumFractionDigits = this._minimumFractionDigits
            newOptions.minimumIntegerDigits = this._minimumIntegerDigits
            newOptions.minimumSignificantDigits = this._minimumSignificantDigits
            newOptions.notation = this._notation
            newOptions.numberingSystem = this._numberingSystem
            newOptions.signDisplay = this._signDisplay
            newOptions.style = this._style
            newOptions.unit = this._unit
            newOptions.unitDisplay = this._unitDisplay
            newOptions.useGrouping = this._useGrouping
            return newOptions;
        }
    }
}
