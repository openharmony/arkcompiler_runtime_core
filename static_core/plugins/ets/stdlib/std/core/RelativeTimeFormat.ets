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

    export interface ResolvedRelativeTimeFormatOptions {
        locale: string;
        numberingSystem: string;
        numeric: RelativeTimeFormatNumeric;
        style: RelativeTimeFormatStyle;
    }

    export interface RelativeTimeFormatOptions {
        numeric?: RelativeTimeFormatNumeric
        style?: RelativeTimeFormatStyle
        localeMatcher?: RelativeTimeFormatLocaleMatcher;
    }

    class ResolvedRelativeTimeFormatOptionsImpl implements ResolvedRelativeTimeFormatOptions {
        private _locale: string;
        private _numberingSystem: string;
        private _numeric: RelativeTimeFormatNumeric;
        private _style: RelativeTimeFormatStyle;

        constructor(locale: string, numberingSystem: string) {
            this._locale = locale;
            this._numberingSystem = numberingSystem;
            this._numeric = 'always';
            this._style = 'long';
        }

        get locale(): string {
            return this._locale;
        }

        set locale(value: string) {
            this._locale = value;
        }

        get numberingSystem(): string {
            return this._numberingSystem;
        }

        set numberingSystem(value: string) {
            this._numberingSystem = value;
        }

        get numeric(): RelativeTimeFormatNumeric {
            return this._numeric;
        }

        set numeric(value: RelativeTimeFormatNumeric) {
            this._numeric = value;
        }

        get style(): RelativeTimeFormatStyle {
            return this._style;
        }

        set style(value: RelativeTimeFormatStyle) {
            this._style = value;
        }
    }

    export interface RelativeTimeFormatPart {
        type: string;
        unit?: RelativeTimeFormatUnit;
        value: string;
    }

    export class RelativeTimeFormat {
        private readonly locale: string | undefined
        private readonly options: ResolvedRelativeTimeFormatOptions

        constructor(locales?: string | string[], options?: RelativeTimeFormatOptions) {
            if (locales !== undefined) {
                const useLookup = options?.localeMatcher === 'lookup'
                this.locale = useLookup ? intlLookUpLocale(locales) : intlBestFitLocale(locales)
            } else {
                this.locale = undefined
            }
            this.options = new ResolvedRelativeTimeFormatOptionsImpl(this.getLocale(), "latn")
            this.options.numeric = options?.numeric ?? "always"
            this.options.style = options?.style ?? "long"
        }

        public static $_invoke(locales?: string | string[], options?: RelativeTimeFormatOptions): RelativeTimeFormat {
            return new RelativeTimeFormat(locales, options)
        }

        format(value: number, unit: RelativeTimeFormatUnit): string {
            return this.formatImpl(value, unit)
        }

        formatToParts(value: number, unit: RelativeTimeFormatUnit): RelativeTimeFormatPart[] {
            let val = this.formatImpl(value, unit)
            let part: RelativeTimeFormatPart = {type: 'literal', value: val}
            return [part]
        }

        resolvedOptions(): ResolvedRelativeTimeFormatOptions {
            return this.options
        }

        static supportedLocalesOf(locales: string | string[], options?: RelativeTimeFormatOptions): string[] {
            return RelativeTimeFormat.supportedLocalesOfImpl(
                (locales instanceof string) ? [locales] : locales, 
                options ?? { numeric: "always", style: "long" } as RelativeTimeFormatOptions
            )
        }

        private native formatImpl(value: number, unit: string): string
        private native getLocale(): string

        private static supportedLocalesOfImpl(locales: string[], options: RelativeTimeFormatOptions): string[] {
            return options.localeMatcher === "lookup" ? intlLookUpLocales(locales) : intlBestFitLocales(locales)
        }
    }
}
