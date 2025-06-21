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

    /**
     * Options for creating a DisplayNames object
     */
    export interface DisplayNamesOptions {
        /** How to handle code fallbacks when no display name is available */
        fallback?: DisplayNamesFallback;
        /** How to display language names (only applies when type is "language") */
        languageDisplay?: DisplayNamesLanguageDisplay;
        /** Strategy for locale matching */
        localeMatcher?: RelativeTimeFormatLocaleMatcher;
        /** Display style for names (narrow, short, or long) */
        style?: RelativeTimeFormatStyle;
        /** Type of code to be displayed (required) */
        type: DisplayNamesType;
    }

    /**
     * Options for locale matching when using supportedLocalesOf
     */
    export interface DisplayNamesLocaleMatcherOptions {
        /** Strategy for locale matching */
        localeMatcher?: RelativeTimeFormatLocaleMatcher;
    }

    /**
     * Resolved options returned by the resolvedOptions method
     */
    export interface ResolvedDisplayNamesOptions {
        /** Current fallback setting */
        fallback: DisplayNamesFallback;
        /** Current language display setting (only present when type is "language") */
        languageDisplay?: DisplayNamesLanguageDisplay;
        /** Resolved locale identifier */
        locale: UnicodeBCP47LocaleIdentifier;
        /** Current display style */
        style: RelativeTimeFormatStyle;
        /** Current type of codes being displayed */
        type: DisplayNamesType;
    }

    /**
     * Format and display locale-sensitive names such as languages, regions, scripts, etc.
     */
    export class DisplayNames {
        /** The resolved locale for this instance */
        private locale: string;
        /** The display style (narrow, short, or long) */
        private style: RelativeTimeFormatStyle;
        /** The type of code to display */
        private type: DisplayNamesType;
        /** How to handle fallbacks */
        private fallback: DisplayNamesFallback;
        /** How to display language names */
        private languageDisplay?: DisplayNamesLanguageDisplay;


        private native static ofNative(locale: string, type: string, code: string, style: string,
                               fallback: string, languageDisplay?: string): string;

        /**
         * Creates a new DisplayNames object
         *
         * @param locales The locale or array of locales to use
         * @param options Configuration options
         *
         * @throws TypeError If options is undefined or type is not specified
         * @throws RangeError If an invalid locale or option value is provided
         */
        public constructor(
            locales?: BCP47LanguageTag | BCP47LanguageTag[],
            options?: DisplayNamesOptions
        ) {
            if (options === undefined) {
                throw new TypeError("Options parameter is required");
            }

            if (options.type === undefined) {
                throw new TypeError("Option 'type' is required");
            }

            // Set default values and process options
            const localeMatcher = options.localeMatcher ?? 'best fit';
            this.style = options.style ?? 'long';
            this.type = options.type;
            this.fallback = options.fallback ?? 'code';

            // Only set languageDisplay if type is 'language'
            if (this.type === 'language') {
                this.languageDisplay = options.languageDisplay ?? 'dialect';
            }

            if (locales !== undefined) {
                const lookupLocale = options != null && options!.localeMatcher == "lookup"
                this.locale = lookupLocale ? intlLookUpLocale(locales) : intlBestFitLocale(locales)
            } else {
                this.locale = Locale.defaultTag();
            }
        }

        /**
         * Returns the localized display name for the provided code
         *
         * @param code The code to get the display name for
         * @returns The localized display name, the code itself (if fallback is 'code'), or undefined
         *
         * @throws RangeError If the code is invalid for the current type
         */
        public of(code: string): string | undefined {
            if (code.isEmpty()) {
                throw new RangeError("Code cannot be empty");
            }
            try {
                const result = DisplayNames.ofNative(
                    this.locale,
                    this.type,
                    code,
                    this.style,
                    this.fallback,
                    this.languageDisplay
                );

                return result;
            } catch (e) {
                if (e instanceof RangeError) throw e;
                throw new RangeError(`Invalid code "${code}" for type "${this.type}"`);
            }
        }

        /**
         * Returns the resolved options for this DisplayNames instance
         *
         * @returns The resolved options object
         */
        public resolvedOptions(): ResolvedDisplayNamesOptions {
            const result: ResolvedDisplayNamesOptions = {
                locale: this.locale,
                style: this.style,
                type: this.type,
                fallback: this.fallback
            };

            // Only include languageDisplay if type is 'language'
            if (this.type === 'language' && this.languageDisplay !== undefined) {
                result.languageDisplay = this.languageDisplay;
            }

            return result;
        }

        /**
        * Returns an array of supported locales for text segmentation
        *
        * @param locales An array of BCP 47 language tags to check
        * @param options Optional configuration for locale matching
        * @returns An array of supported locales from the input
        */
        public static supportedLocalesOf(locales: string | Locale | FixedArray<string | Locale>): string[] {
            const supportedLocales = new Array<string>()

            if (locales instanceof FixedArray<string>) {
                return DisplayNames.bestFitLocales(Array.from(locales))
            } else if (locales instanceof FixedArray<Locale>) {
                return DisplayNames.bestFitLocales(Array.from(locales, (loc: Locale) => loc.toString()))
            } else if (locales instanceof string) {
                return DisplayNames.bestFitLocales(Array.of(locales))
            } else if (locales instanceof Locale) {
                return DisplayNames.bestFitLocales(Array.of(locales).map((loc: Locale) => loc.toString()))
            } else {
                throw new TypeError("'locales' parameter has invalid type")
            }
        }

        private static bestFitLocales(locales: Array<string>): string[] {
            return intlBestFitLocales(locales)
        }
    }
}
