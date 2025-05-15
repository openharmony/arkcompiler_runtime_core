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
    static native lookupLocale(locale: string[]): string
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

export function intlLocalesToLanguageTags(locales: string | Intl.Locale | ReadonlyArray<string | Intl.Locale> | undefined): string[] {
    if (locales === undefined) {
        return []
    }

    if (locales instanceof string) {
        return [locales]
    } else if (locales instanceof Intl.Locale) {
        return [locales.toString()]
    } else if (locales instanceof ReadonlyArray) {
        if (locales.length == 0) {
            return []
        }

        const langTags = new string[locales.length]
        for (let i = 0; i < locales.length; i++) {
            const locale = locales[i] as string | Intl.Locale
            langTags[i] = locale instanceof string ? locale : locale.toString()
        }
        return langTags
    } else {
        throw new TypeError("'locales' parameter has invalid type")
    }
}

function supportedLocalesOf(locales: string[], localeFitFn: (loc: string) => string): string[] {
    const supportedLocales = new Array<string>()

    for (const locale of locales) {
        const fitLocale = localeFitFn(locale)
        if (locale.startsWith(fitLocale)) {
            supportedLocales.push(locale)
        }
    }

    return supportedLocales
}

export function intlSupportedBestFitLocales(locales: string[]): string[] {
    return supportedLocalesOf(locales, intlBestFitLocale)
}

export function intlSupportedLookupLocales(locales: string[]): string[] {
    return supportedLocalesOf(locales, intlLookUpLocale)
}
