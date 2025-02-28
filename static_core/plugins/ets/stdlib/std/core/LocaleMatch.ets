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

export class LocaleMatch {
    native static bestFitLocale(locale: string): string
    native static lookupLocale(locale: string[]): string
}

export function localesToString(locale: Intl.BCP47LanguageTag[]): string {
    return string.join(locale, ",")
}

export function localesToString(locale: Intl.BCP47LanguageTag): string {
    return locale
}

export function localesToString(locale: Intl.BCP47LanguageTag | Intl.BCP47LanguageTag[]): string {
    if (locale instanceof Intl.BCP47LanguageTag) {
        return locale
    } else {
        return localesToString(locale)
    }
}

export function intlLookUpLocale(locale: Intl.BCP47LanguageTag | Intl.BCP47LanguageTag[]): string {
    let tagList: string[]
    if (locale instanceof Intl.BCP47LanguageTag) {
        tagList = [locale]
    } else {
        tagList = locale
    }
    return LocaleMatch.lookupLocale(tagList)
}

export function intlBestFitLocale(locale: Intl.BCP47LanguageTag | Intl.BCP47LanguageTag[]): string {
    return LocaleMatch.bestFitLocale(localesToString(locale))
}
