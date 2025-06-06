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
desc: >-
    Type aliases provide names for anonymous types (array, function or nullable types)
    or alternative names for existing types.
---*/

type Matrix = number[][]
type Handler = (s: string, no: number) => string
type Predicate<T> = (x: T) => Boolean
type NullableNumber = Number | null

type Dictionary = Map<string, string>
type MapOfString<T> = Map<T, string>

type Vector = number[]

function max(x: Vector): number {
    let m = x[0]
    for (let v of x)
        if (v > m) m = v
    return m
}

function main(): int {
    let x: Vector = [1, 2, 3]
    return max(x) == 3 ? 0 : 1
}
