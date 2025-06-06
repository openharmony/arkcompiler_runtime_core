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
desc: Implementing property from interface. Sample from specification
assert: >-
    If a property is defined in the readonly form, then the implementation of the property can
    either keep the readonly form or extend it to a writable form as follows
tags: [compile-only, negative]
---*/

interface Style {
    readonly readable: number
    get color(): string
}

class StyleClassThree implements Style {
    get color(): string { return "Black" }
    set color(s: string) {} // OK!
    readable: number = 0 // OK!
}

function how_to_write (s: Style) {
    s.color = "Black" // compile-time error
    s.readable = 666 // compile-time error
}

function main() {
    how_to_write (new StyleClassThree)
}
