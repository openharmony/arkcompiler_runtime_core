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

    const SensitivityValues : ReadonlyArray<string> = Array.of<string>("base",  "accent", "case", "variant")
    const UsageValues : ReadonlyArray<string> = Array.of<string>("sort", "search")
    const CaseFirstValues : ReadonlyArray<string> = Array.of<string>("upper", "lower", "false")
    const LocaleMatcherValues : ReadonlyArray<string>  = Array.of<string>("lookup", "best fit")

    export interface ResolvedCollatorOptions {
        caseFirst: string
        collation: string
        ignorePunctuation: boolean
        locale: string
        numeric: boolean
        sensitivity: string
        usage: string
    }

    export interface CollatorOptions {
        caseFirst?: string | undefined
        collation?: string | undefined
        ignorePunctuation?: boolean | undefined
        localeMatcher?: string | undefined
        numeric?: boolean | undefined
        sensitivity?: string | undefined
        usage?: string | undefined
    }

    function errorMessage(name: string, value: string) : string {
        return "Value \"" + value + "\" out of range for Intl.Collator options property \"" + name + "\""
    }

    function checkOptions(options: CollatorOptions) {
        if ((options.caseFirst != undefined) && !CaseFirstValues.includes(options.caseFirst!)) {
            throw new RangeError(errorMessage("caseFirst", options.caseFirst!))
        }

        if ((options.sensitivity != undefined) && !SensitivityValues.includes(options.sensitivity!)) {
            throw new RangeError(errorMessage("sensitivity", options.sensitivity!))
        }

        if ((options.usage != undefined) && !UsageValues.includes(options.usage!)) {
            throw new RangeError(errorMessage("usage", options.usage!))
        }

        if ((options.localeMatcher != undefined) && !LocaleMatcherValues.includes(options.localeMatcher!)) {
            throw new RangeError(errorMessage("localeMatcher", options.localeMatcher!))
        }
    }

    export class Collator {
        /**
        * Compares 2 input strings x and y
        *
        * @param x input string to compare
        * @param y input string to compare
        * @return 1 if x "more" than y
        * @return -1 if y "more" than x
        * @return 0 if x equal y
        */
        public compare(x: string, y: string): number {
            if (x == y) {
                return 0
            }

            if (this.options.numeric) {
                let xNumber = new Number(x.replaceAll(',', '.'))
                let yNumber = new Number(y.replaceAll(',', '.'))

                if (!isNaN(xNumber) && !isNaN(yNumber)) {
                    if (xNumber == yNumber) {
                        return 0
                    }

                    // Intl.Collator comapres negative numbers by abs but numbers with different signs correct
                    // https://stackoverflow.com/questions/73939121/why-does-intl-collator-sort-negative-numbers-in-descending-order
                    if (xNumber < 0 && yNumber < 0) {
                        xNumber = abs(xNumber)
                        yNumber = abs(yNumber)
                    }
                    return xNumber > yNumber ? 1 : -1;
                }
            }

            let xToCompare : string = x
            let yToCompare : string = y

            if (this.options.ignorePunctuation) {
                xToCompare = this.removePunctuation(xToCompare)
                yToCompare = this.removePunctuation(yToCompare)
            }

            switch (this.options.sensitivity) {
                case "base": {
                    xToCompare = xToCompare.toLowerCase()
                    yToCompare = yToCompare.toLowerCase()

                    xToCompare = this.removeAccents(xToCompare)
                    yToCompare = this.removeAccents(yToCompare)
                }
                break;
                case "accent": {
                    xToCompare = xToCompare.toLowerCase()
                    yToCompare = yToCompare.toLowerCase()
                }
                break;
                case "case": {
                    xToCompare = this.removeAccents(xToCompare)
                    yToCompare = this.removeAccents(yToCompare)
                }
                break;
                case "variant":
                break;
            }

            if (xToCompare == yToCompare) {
                return 0
            }

            if (this.options.usage == "search") {
                return 1
            }

            const IgnoreSensitivityValues : ReadonlyArray<string> = Array.of<string>("base",  "accent")
            if (!IgnoreSensitivityValues.includes(this.options.sensitivity)) {
                let lowerXToCompare = xToCompare.toLowerCase()
                let lowerYToCompare = yToCompare.toLowerCase()
                if (lowerXToCompare == lowerYToCompare) {
                    let i: int = 0
                    while (xToCompare[i] == yToCompare[i]) {
                        ++i
                    }

                    if (lowerXToCompare[i] == xToCompare[i] || yToCompare.toUpperCase()[i] == yToCompare[i]) {
                        return (this.options.caseFirst == "upper") ? 1 : -1
                    }

                    return (this.options.caseFirst == "upper") ? -1 : 1
                }
            }
            return this.compareByCollation(this.options.collation, this.options.locale, xToCompare, yToCompare)
        }

        /**
        * The Intl.Collator() constructor creates Intl.Collator objects.
        *
        * @param locales locales
        * @param options options
        */
        public constructor(locales?: string | string[], options?: CollatorOptions) {
            this.options = {caseFirst: "false", collation: "default", ignorePunctuation: false,
            locale: Locale.defaultTag(),
            numeric: false,
            sensitivity: "variant",
            usage: "sort"}

            this.options.collation = "default"
            try {
                let l = Collator.supportedLocalesOf((locales == undefined) ? "" : locales, options)
                if (l.length > 0) {
                    this.options.locale = l[0]
                    let locale = new Intl.Locale(l[0])
                    this.options.collation = locale.collation != undefined ? locale.collation! : this.options.collation
                    this.options.numeric = locale.numeric != undefined ? locale.numeric! : this.options.numeric
                    this.options.caseFirst = locale.caseFirst != undefined ? locale.caseFirst! : this.options.caseFirst
                }
            }
            catch(e) {
                throw new RangeError("Incorrect locales " + locales);
            }

            if (options != undefined) {
                checkOptions(options!)
                this.options.caseFirst = (options.caseFirst == undefined) ? this.options.caseFirst : options.caseFirst!
                this.options.ignorePunctuation = (options.ignorePunctuation == undefined) ? false : options.ignorePunctuation!
                this.options.numeric = (options.numeric == undefined) ? this.options.numeric : options.numeric!
                this.options.sensitivity = (options.sensitivity == undefined) ? "variant" : options.sensitivity!
                this.options.usage = (options.usage == undefined) ? "sort" : options.usage!
                this.options.collation = (options.collation == undefined) ? this.options.collation : options.collation!
            }
        }

        /**
        * Returns resolved options
        * @return resolved options
        */
        public resolvedOptions(): ResolvedCollatorOptions {
            return this.options;
        }

        /**
        * Returns supported locales
        * @param locales locales
        * @param options options
        * @return supported locales
        */
        static supportedLocalesOf(locales: string | string[], options?: CollatorOptions): string[] {
            if (options != undefined) {
                checkOptions(options!)
            }
            let lookup = (options != undefined) ? options.localeMatcher == "lookup" : false
            try {
                return lookup ? intlLookUpLocales(locales) : intlBestFitLocales(locales)
            }
            catch(e) {
                if (e instanceof Error) {
                    throw e
                }
                else {
                    throw new RangeError("Incorrect locales " + locales)
                }
            }
        }

        private readonly options : ResolvedCollatorOptions

        private native removePunctuation(text: string) : string
        private native removeAccents(text: string) : string

        /**
        * Returns icu-collator compare result for 2 input strings  x and y in case with non-default collation for given lang
        * @param data string array contains collation, lang, x, y
        * @return icu-collator 2 strings compare result: 1 (x "more" than y), -1 (y "more" than x) or 0 (equals)
        */
        private native compareByCollation(collation : string, lang: string, str1: string, str2: string) : double
    }
}
