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
name: 03.types/08.reference_types/03.08.11.union_types/union_compare_03
tags: []
---*/
type Margin = number | string

class A {}

function main(): void {
    let a: Margin = 5
    let b: Margin = 6
    assertTrue( (a as number) < b as number)
    assertTrue( b as number > a as number)

    let c: Margin = 5
    assertTrue( a as number <= c as number)
    assertTrue( c as number >= a as number)
}
