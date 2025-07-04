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

package std.core

export namespace Intl {

    export interface ListFormatOptions {
        localeMatcher?: ListFormatLocaleMatcher | undefined
        style?: ListFormatStyle | undefined
        type?: ListFormatType | undefined
    }

    export interface FormatToPartsResult {
        type: "element" | "literal"
        value: string
    }
    
    export class ListFormat {
        private locale: string
        private style: ListFormatStyle
        private type: ListFormatType

        public constructor(locales?: string | string[], options?: ListFormatOptions) {
            this.style = options?.style ?? "long"
            this.type = options?.type ?? "conjunction"
            this.locale = this.resolveLocale(locales, options?.localeMatcher)
        }

        public format(list: Iterable<string>): string {
            const parts = this.formatToParts(list)
            return parts.map((part: FormatToPartsResult) => part.value).join('')
        }

        public formatToParts(list: Iterable<string>): Array<FormatToPartsResult> {
            const arr = ListFormat.iterableToArray(list)
            if (arr.length == 0) {
                return new Array<FormatToPartsResult>()
            }
            const squashed = ListFormat.formatToPartsNative(arr, this.locale, this.style, this.type) as string[]
            if (squashed.length % 2 != 0) {
                throw Error("ListFormat.formatToPartsNative incorrect result")
            }
            const result = new Array<FormatToPartsResult>(squashed.length / 2)
            for (let i = 0; i < result.length; i++) {
                result[i] = {
                    type: squashed[i * 2] as "element" | "literal",
                    value: squashed[i * 2 + 1]
                } as FormatToPartsResult
            }
            return result
        }

        public static supportedLocalesOf(locales: string | string[], options?: ListFormatLocaleMatcher): string[] {
            return options === "lookup" ? intlLookUpLocales(locales) : intlBestFitLocales(locales)
        }

        private resolveLocale(locales?: string | string[], localeMatcher?: ListFormatLocaleMatcher): string {
            if (locales === undefined) {
                return Locale.defaultTag()
            }
            return localeMatcher === "lookup" ? intlLookUpLocale(locales!) : intlBestFitLocale(locales!)
        }

        private static iterableToArray(list: Iterable<string>): string[] {
            const arr = new Array<string>()

            const it = list.$_iterator()
            for (let itRes = it.next(); !itRes.done; itRes = it.next()) {
                const item = itRes.value
                if (item !== undefined) {
                    arr.push(item)
                }
            }
            
            const result = new string[arr.length]
            for (let i = 0; i < arr.length; i++) {
                result[i] = arr[i]
            }
            return result
        }

        private static native formatToPartsNative(
            list: string[], locale: string, style: string, type: string): Object
    }

}
