/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
    03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_generics_01
tags: []
---*/
type Margin = number | string

function superMax<T>(items: FixedArray<T>, comparator: ((l: T, r: T) => int) | null = null): T | null {
    if (comparator != null && items.length > 0) {
        let max: T = items[0]
        for (const item: T of items) {
            if (comparator(max, item) < 0) {
                max = item
            }
        }
        return max
    }
    return null
}

function getNumber(param: Margin): int {
    if (param instanceof string) {
        return (param as string).length as int
    }
    return (param as number) as int
}

function compareMargin(left: Margin, right: Margin): int {
    let leftN: int = getNumber(left)
    let rightN: int = getNumber(right)
    return leftN - rightN
}

function main(): void {
    let arr: FixedArray<Margin> = [5, "hello", "bye", 7]
    let maxx = superMax<Margin>(arr, compareMargin)
    assertEQ( (maxx as number), 7)
}
