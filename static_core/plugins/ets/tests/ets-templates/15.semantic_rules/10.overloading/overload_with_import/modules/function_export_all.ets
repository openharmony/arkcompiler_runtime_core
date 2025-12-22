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
desc: Functions to import to another test
tags:
- compile-only
- not-a-test
---*/

export function foo01(): int {
    return 1;
}

export function foo02(i: boolean): int {
    return 2;
}

export function foo03(i: short): int {
    return 3;
}

export function foo04(i: int): int {
    return 4;
}

export function foo05(i: long): int {
    return 5;
}

export function foo06(i: float): int {
    return 6;
}

export function foo07(i: double): int {
    return 7;
}

export function foo08(i: string): int {
    return 8;
}

export function foo09(i: Object): int {
    return 9;
}

export function foo10(i: number[]): int {
    return 10;
}
