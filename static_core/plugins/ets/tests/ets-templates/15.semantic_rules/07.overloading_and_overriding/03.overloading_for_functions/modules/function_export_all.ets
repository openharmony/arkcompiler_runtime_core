/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
desc: Functions to import to another test
tags:
- compile-only
- not-a-test
---*/

export type TPL =  [Number, String];

export function foo(): int {
    return 1;
}

export function foo(i: boolean): int {
    return 2;
}

export function foo(i: Boolean): int {
    return 3;
}

export function foo(i: char): int {
    return 4;
}

export function foo(i: Char): int {
    return 5;
}

export function foo(i: short): int {
    return 6;
}

export function foo(i: Short): int {
    return 7;
}

export function foo(i: int): int {
    return 8;
}

export function foo(i: Int): int {
    return 9;
}

export function foo(i: long): int {
    return 10;
}

export function foo(i: Long): int {
    return 11;
}

export function foo(i: float): int {
    return 12;
}

export function foo(i: Float): int {
    return 13;
}

export function foo(i: double): int {
    return 14;
}

export function foo(i: Number): int {
    return 15;
}

export function foo(i: string): int {
    return 16;
}

export function foo(i: Object): int {
    return 17;
}

export function foo(i: Number[]): int {
    return 18;
}

export function foo(i: TPL): int {
    return 19;
}

export function foo<T,S>(i: T, j: S): int {
    return 20;
}

export function foo(i: () => boolean ): int {
    return 21;
}

export function foo(i: int, j: double, k?: String ): int {
    return 22;
}
