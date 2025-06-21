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

    const calendarValues: ReadonlyArray<string> = Array.of<string> (
        "buddhist",
        "chinese",
        "coptic",
        "dangi",
        "ethioaa",
        "ethiopic",
        "gregory",
        "hebrew",
        "indian",
        "islamic",
        "islamic-umalqura",
        "islamic-tbla",
        "islamic-civil",
        "islamic-rgsa",
        "iso8601",
        "japanese",
        "persian",
        "roc",
        "islamicc"
    )

    const extensionValues = [
        "ca",   // calendar
        "kf",   // caseFirst
        "co",   // collation
        "hc",   // hourCycle
        "nu",   // numberingSystem
        "kn"    // numeric
    ]

    const caseFirstValues: ReadonlyArray<string> = Array.of<string>("upper", "lower", "false")
    const hourCycleValues: ReadonlyArray<string> = Array.of<string>("h12", "h23", "h11", "h24")

    const collationValues: ReadonlyArray<string> = Array.of<string>(
        "big5han",
        "compat",
        "dict",
        "direct",
        "ducet",
        "emoji",
        "eor",
        "gb2312",
        "phonebk",
        "phonetic",
        "pinyin",
        "reformed",
        "search",
        "searchjl",
        "standard",
        "stroke",
        "trad",
        "unihan",
        "zhuyin"
    )

    enum LocaleInfoKeys {
        LANG,
        SCRIPT,
        COUNTRY,
        U_CA,
        U_KF,
        U_CO,
        U_HC,
        U_NU,
        U_KN,
        COUNT
    }

    export interface LocaleOptions {
        baseName?: string
        language?: string
        calendar?: string
        caseFirst?: string // LocaleCollationCaseFirst
        numeric?: boolean
        collation?: string
        hourCycle?: string  // LocaleHourCycleKey
        numberingSystem?: string
        region?: string
        script?: string
    }

    export class Locale implements LocaleOptions {
        private _baseName?: string;
        private _language?: string;
        private _calendar?: string;
        private _caseFirst?: string;
        private _numeric?: boolean;
        private _collation?: string;
        private _hourCycle?: string;
        private _numberingSystem?: string;
        private _region?: string;
        private _script?: string;

        set baseName(value: string | undefined) {
            this._baseName = value
        }

        get baseName(): string | undefined {
            return this._baseName
        }

        set language(value: string | undefined) {
            this._language = value
        }

        get language(): string | undefined {
            return this._language
        }

        set calendar(value: string | undefined) {
            this._calendar = value
        }

        get calendar(): string | undefined {
            return this._calendar
        }

        set caseFirst(value: string | undefined) {
            this._caseFirst = value
        }

        get caseFirst(): string | undefined {
            return this._caseFirst
        }

        set numeric(value: boolean | undefined) {
            this._numeric = value
        }

        get numeric(): boolean | undefined {
            return this._numeric
        }

        set collation(value: string | undefined) {
            this._collation = value
        }

        get collation(): string | undefined {
            return this._collation
        }

        set hourCycle(value: string | undefined) {
            this._hourCycle = value
        }

        get hourCycle(): string | undefined {
            return this._hourCycle
        }

        set numberingSystem(value: string | undefined) {
            this._numberingSystem = value
        }

        get numberingSystem(): string | undefined {
            return this._numberingSystem
        }

        set region(value: string | undefined) {
            this._region = value
        }

        get region(): string | undefined {
            return this._region
        }

        set script(value: string | undefined) {
            this._script = value
        }

        get script(): string | undefined {
            return this._script
        }

        /**
        * The Intl.Locale() constructor creates Intl.Locale objects.
        *
        * @param tag BCP47LanguageTag or other Locale
        * @param options options
        */
        public constructor(tag: BCP47LanguageTag | Locale, options?: LocaleOptions) {
            this.initLocale()
            this.language = this.defaultLang()
            if (tag instanceof BCP47LanguageTag) {
                this.tag = tag
                this.parseTag()
            }
            else {
                this.language = (tag.language != undefined) ? tag.language : this.defaultLang()
                this.baseName = (tag.baseName != undefined) ? tag.baseName : this.defaultLang()
                this.calendar = (tag.calendar != undefined) ? tag.calendar : ""
                this.caseFirst = (tag.caseFirst != undefined) ? tag.caseFirst : undefined
                this.numeric = (tag.numeric != undefined) ? tag.numeric : false
                this.collation = tag.collation
                this.hourCycle = tag.hourCycle
                this.numberingSystem = tag.numberingSystem
                this.region = tag.region
                this.script = tag.script
                this.tag = tag.tag
            }
            if (options != undefined) {
                this.checkOptions(options)
                this.language = (options.language != undefined) ? options.language : this.defaultLang()
                this.baseName = (options.baseName != undefined) ? options.baseName : this.defaultLang()
                this.calendar = (options.calendar != undefined) ? options.calendar : ""
                this.caseFirst = (options.caseFirst != undefined) ? options.caseFirst : undefined
                this.numeric = (options.numeric != undefined) ? options.numeric : false
                this.collation = (options.collation != undefined) ? options.collation : this.collation
                this.hourCycle = (options.hourCycle != undefined) ? options.hourCycle : this.hourCycle
                this.numberingSystem = (options.numberingSystem != undefined) ? options.numberingSystem : this.numberingSystem
                this.region = (options.region != undefined) ? (options.region as string).toUpperCase() : this.region
                this.script = (options.script != undefined) ? options.script : this.script
            }

            if (this.region != undefined && !this.regionList().includes((this.region as string).toUpperCase())) {
                throw new RangeError(this.errMsg)
            }

            if (this.script != undefined && !this.scriptList().includes(this.script as string)) {
                throw new RangeError(this.errMsg)
            }

            this.tag = this.createTag()
        }

        native langList() : string
        native regionList() : string
        native scriptList() : string
        native numberingSystemList() : string
        native initLocale() : void
        native maximizeInfo(lang: string) : string
        native isTagValid(tag: string) : int
        native parseTagImpl(tag:string) : string
        private native defaultLang() : string

        static native defaultTag(): string

        private fillBaseName() {
            let sb = new StringBuilder()
            sb.append(this.language as string)
            if (this.script != undefined && this.script != "" && this.script != " ") {
                sb.append("-")
                sb.append(this.script as string)
            }
            if (this.region != undefined && this.region != "" && this.region != " ") {
                sb.append("-")
                sb.append(this.region as string)
            }
            this.baseName = sb.toString()
        }

        /**
        * The maximize() method of Intl.Locale instances gets the most likely values for the language,
        * script, and region of this locale based on existing values.
        * @return A Intl.Locale instance whose baseName property returns the result of the
        * Add Likely Subtags algorithm executed against locale.baseName.
        */
        public maximize() : Locale {
            let res = new Locale(this)
            let info = this.maximizeInfo(this.language as string).split(",")
            res.region = info[0]
            res.script = info.length > 1 ? info[1] : res.script
            this.fillBaseName()
            res.tag = res.createTag()
            return res
        }

        /**
        * The minimize() attempts to remove information about this locale that would be added by calling maximize().
        * @return A Intl.Locale instance whose baseName property returns the result of the
        * Remove Likely Subtags algorithm executed against locale.baseName.
        */
        public minimize() : Locale {
            let res = new Locale(this)
            res.region = undefined
            res.script = undefined
            res.baseName = res.language
            res.tag = res.createTag()
            return res
        }

        /**
        * The toString() method of Intl.Locale instances returns this Locale's full locale identifier string.
        * @return The locale's Unicode locale identifier string.
        */
        public toString() : BCP47LanguageTag {
            return this.tag
        }

        private tag: BCP47LanguageTag = ""

        private uFlags() : BCP47LanguageTag {
            let sb = new StringBuilder()
            if ((this.numeric != undefined && this.numeric as boolean == true)) {
                sb.append("-u-kn")
            }
            if (this.caseFirst != undefined && this.caseFirst != "") {
                sb.append("-u-kf-" + this.caseFirst as string)
            }
            if (this.calendar != undefined && this.calendar != "") {
                sb.append("-u-ca-" + this.calendar as string)
            }
            if (this.collation != undefined && this.collation != "") {
                sb.append("-u-co-" + this.collation as string)
            }
            if (this.hourCycle != undefined && this.hourCycle != "" && (this.hourCycle instanceof LocaleHourCycleKey)) {
                sb.append("-u-hc-" + this.hourCycle as string)
            }
            if (this.numberingSystem != undefined && this.numberingSystem != "") {
                sb.append("-u-nu-" + this.numberingSystem as string)
            }
            return sb.toString()
        }

        private createTag() : BCP47LanguageTag {
            let sb = new StringBuilder()
            this.fillBaseName()
            sb.append(this.baseName as string)
            sb.append(this.uFlags())
            return sb.toString()
        }

        private errOptionMessage(val : string, name : string) : string {
            return "Value " + val + " out of range for locale options property " + name
        }

        private errMsg = "Incorrect locale information provided"
        private checkOptions(options: LocaleOptions) {
            if  ((options.calendar != undefined && !calendarValues.includes(options.calendar!)) ||
                (options.caseFirst != undefined && !caseFirstValues.includes(options.caseFirst!)) ||
                (options.collation != undefined && !collationValues.includes(options.collation!)) ||
                (options.hourCycle != undefined && !hourCycleValues.includes(options.hourCycle!)) ||
                (options.numberingSystem != undefined && (options.numberingSystem! == "" ||
                                !this.numberingSystemList().includes(options.numberingSystem!))) ||
                (options.region != undefined && (options.region! == "" || !this.regionList().includes((options.region! as String).toUpperCase()))) ||
                (options.script != undefined && (options.script! == "" || !this.scriptList().includes(options.script!)))
            ) {
                throw new RangeError(this.errMsg)
            }
        }

        private parseTag() {
            if (this.tag == "") {
                throw new RangeError("First argument to Intl.Locale constructor can't be empty or missing")
            }

            if (this.isTagValid(this.tag as string) == 1) {
                throw new RangeError(this.errMsg)
            }

            let argsStr : string = this.parseTagImpl(this.tag)
            let args = argsStr.split(" ")
            let getVale = (val: string) : string|undefined => {return (val == "") ? undefined : val}
            this.language = args[LocaleInfoKeys.LANG as int]
            this.script = getVale(args[LocaleInfoKeys.SCRIPT as int])
            this.region = getVale(args[LocaleInfoKeys.COUNTRY as int])
            this.calendar = getVale(args[LocaleInfoKeys.U_CA as int])
            this.collation = getVale(args[LocaleInfoKeys.U_CO as int])
            this.numberingSystem = getVale(args[LocaleInfoKeys.U_NU as int])
            this.hourCycle = getVale(args[LocaleInfoKeys.U_HC as int])
            this.numeric = (args[LocaleInfoKeys.U_KN as int] == "true") ? true : false
            this.caseFirst = getVale(args[LocaleInfoKeys.U_KF as int])

            if (this.tag.includes("-u")) {
               if (this.language != undefined && !this.langList().includes(this.language as string)) {
                    throw new RangeError("Incorrect locale information provided")
               }
            }
            this.fillBaseName()
        }

        private fillScriptOrRegion(val : string) {
            if (val.length <= 3) {
                this.region = val.toUpperCase()
            }
            else {
                this.script = val
            }
        }
    }
}
