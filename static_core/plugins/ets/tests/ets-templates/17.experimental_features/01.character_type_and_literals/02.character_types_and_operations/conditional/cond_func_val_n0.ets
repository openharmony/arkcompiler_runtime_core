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

/*---
desc: >-
    Widening Primitive Conversion converts cher to int, so char + char => int
tags: [compile-only, negative]
---*/

function foo(c: boolean, p: char, q: char): char {
    return c ? p : q
}

function main() {
    const v1 = c'\u0020'
    const v2 = c'\u0021'

    assertEQ( (c' ' + foo(false, v2 + v1, v2 - v1)), 1 )
}
