/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

package std.core

export namespace Intl {

    export interface PluralRulesOptions {
        localeMatcher?: "lookup" | "best fit" | undefined
        type?: PluralRuleType | undefined
        minimumIntegerDigits?: number | undefined
        minimumFractionDigits?: number | undefined
        maximumFractionDigits?: number | undefined
        minimumSignificantDigits?: number | undefined
        maximumSignificantDigits?: number | undefined
    }

    export interface ResolvedPluralRulesOptions {
        locale: string
        localeMatcher: "lookup" | "best fit"
        type: PluralRuleType
        minimumIntegerDigits: number
        minimumFractionDigits: number
        maximumFractionDigits: number
        minimumSignificantDigits?: number
        maximumSignificantDigits?: number
        pluralCategories: LDMLPluralRule[]
    }

    export interface SupportedLocalesOfOptions {
        localeMatcher?: "lookup" | "best fit" | undefined
    }

    export class PluralRulesSelectOptions {
        locale: string = ""
        type: string = ""
        minimumIntegerDigits: number
        minimumFractionDigits: number
        maximumFractionDigits: number
        minimumSignificantDigits: number
        maximumSignificantDigits: number
    }
    
    export class PluralRules {
        private options : ResolvedPluralRulesOptions

        public constructor(locales?: string | string[], options?: PluralRulesOptions) {
            const localeMatcher = options?.localeMatcher ?? "best fit"
            const locale = this.resolveLocale(localeMatcher, locales)
            const type: PluralRuleType = options?.type ?? "cardinal"
            const categories = PluralRules.getPluralCategories(locale, type) as LDMLPluralRule[]
            this.options = {
                locale: locale,
                localeMatcher: localeMatcher,
                minimumIntegerDigits: options?.minimumIntegerDigits ?? 1,
                minimumFractionDigits: options?.minimumFractionDigits ?? 0,
                maximumFractionDigits: options?.maximumFractionDigits ?? 3,
                minimumSignificantDigits: options?.minimumSignificantDigits,
                maximumSignificantDigits: options?.maximumSignificantDigits,
                pluralCategories: categories,
                type: type,
            } as ResolvedPluralRulesOptions
        }

        private resolveLocale(localeMatcher: "lookup" | "best fit", locales?: string | string[]): string {
            const locale = PluralRules.toArray(locales)[0]
            return localeMatcher === "lookup" ? intlLookUpLocale(locale) : intlBestFitLocale(locale)
        }

        public resolvedOptions(): ResolvedPluralRulesOptions {
            return this.options
        }

        public select(value: number): LDMLPluralRule {
            const options = {
                locale: this.options.locale,
                type: this.options.type,
                minimumIntegerDigits: this.options.minimumIntegerDigits,
                minimumFractionDigits: this.options.minimumFractionDigits,
                maximumFractionDigits: this.options.maximumFractionDigits,
                minimumSignificantDigits: this.options.minimumSignificantDigits ?? 0,
                maximumSignificantDigits: this.options.maximumSignificantDigits ?? 0
            } as PluralRulesSelectOptions
            return PluralRules.selectDouble(value, options) as LDMLPluralRule
        }

        public static supportedLocalesOf(locales: string | string[], options?: SupportedLocalesOfOptions): string[] {
            const result = PluralRules.toArray(locales)
            const localeMatcher = options?.localeMatcher ?? "best fit"
            for (let i = 0; i < result.length; i++) {
                result[i] = localeMatcher === "lookup"
                    ? intlLookUpLocale(result[i])
                    : intlBestFitLocale(result[i])
            }
            return result
        }

        private static toArray(locales?: string | string[]): string[] {
            if (!locales) {
                return [PluralRules.getDefaultLocaleTag()]
            }
            if (locales instanceof string) {
                return [locales as string]
            }
            return locales as string[]
        }

        private static native selectDouble(value: double, options: Object): string

        private static native getDefaultLocaleTag(): string

        private static native getPluralCategories(locale: string, type: string): Object
    }

}
