/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: Old lib version.
tags: [compile-only, not-a-test]
---*/
export class A<T> {
    public fld: T
    constructor(p: T) {
        this.fld = p
    }
}

// Effective type: Array
export let array: number[] = [1, 2, 3]

// Effective type: FixedArray<int|undefined>
export let fixedArray: FixedArray<int|undefined> = [1, 2, 3, undefined]

// Effective type: Tuple4|undefined
export let tuple: [int, long, short, byte]|undefined = [1, 2, 3, 4]

// Effective type: string
export let stringLiteral: 'abc'|'def'|'ghi' = 'abc'

// Effective type: A
export let obj: Readonly<A<string>> = new A<string>('abc')
