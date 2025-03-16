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

/*---
desc: 10.6.2 Inheritance and Overriding
tags: [compile-only]
---*/

type EntryType = [PropertyKey, NullishType]
type __Record<K, V> = Map<K, V>
type RecordForObject = Map<PropertyKey, NullishType>

interface I {
    keys(o: Object): string[]
    entries(o: Object): EntryType[]
    getOwnPropertyNames(o: Object): string[]
    hasOwn(target: Object, key: string): boolean
    hasOwn(target: Object, index: number): boolean
    values__(o: Object): NullishType[]
    fromEntries<T>(entries: Map<PropertyKey, T>): __Record<PropertyKey, T>
    assign(target: RecordForObject,  ...source: Object[]): RecordForObject
}

