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

    export class RelativeTimeFormatOptions {
        public numeric: RelativeTimeFormatNumeric = "always";
        public style: RelativeTimeFormatStyle = "long";
        public localeMatcher?: RelativeTimeFormatLocaleMatcher;

        constructor(options?: RelativeTimeFormatOptions) {
            if (options !== undefined) {
                this.numeric = options.numeric ?? "always";
                this.style = options.style ?? "long";
                this.localeMatcher = options.localeMatcher;
            }
        }

        public static isEmpty(options: RelativeTimeFormatOptions): boolean {
            return options.numeric === undefined &&
                options.style === undefined &&
                options.localeMatcher === undefined;
        }

        public toString(): string {
            return JSON.stringify(this);
        }
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
        private static readonly DEFAULT_OPTIONS: RelativeTimeFormatOptions = {
            numeric: "always",
            style: "long"
        };
        private readonly locale: string | undefined
        private readonly options: RelativeTimeFormatOptions

        constructor(locales?: string | string[], options?: RelativeTimeFormatOptions) {
            if (locales !== undefined) {
                const useLookup = options?.localeMatcher === 'lookup'
                this.locale = useLookup ? intlLookUpLocale(locales) : intlBestFitLocale(locales)
            } else {
                this.locale = undefined
            }
            const resolved: RelativeTimeFormatOptions = {
                numeric: options?.numeric ?? RelativeTimeFormat.DEFAULT_OPTIONS.numeric,
                style: options?.style ?? RelativeTimeFormat.DEFAULT_OPTIONS.style,
                localeMatcher: options?.localeMatcher
            }

            this.options = resolved
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
            const opt = this.resolvedOptionsImpl()
            opt.numeric = this.options.numeric ?? 'always';
            opt.style = this.options.style ?? 'long';
            return opt
        }

        static supportedLocalesOf(locales: string, options?: RelativeTimeFormatOptions): string[] {
            return RelativeTimeFormat.supportedLocalesOfImpl([locales], options ?? RelativeTimeFormat.DEFAULT_OPTIONS)
        }

        static supportedLocalesOf(locales: string[], options?: RelativeTimeFormatOptions): string[] {
            return RelativeTimeFormat.supportedLocalesOfImpl(locales, options ?? RelativeTimeFormat.DEFAULT_OPTIONS)
        }

        private native formatImpl(value: number, unit: string): string
        private native resolvedOptionsImpl(): ResolvedRelativeTimeFormatOptions

        private static supportedLocalesOfImpl(locales: string[], options: RelativeTimeFormatOptions): string[] {
            return options.localeMatcher === "lookup" ? intlLookUpLocales(locales) : intlBestFitLocales(locales)
        }
    }
}
