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
desc: >-
    Origin library exporting symbols.
    These will be re-exported via export * from.
tags: [not-a-test, compile-only]
name: lib_origin
arktsconfig: arktsconfig-origin.json
es2panda_options: ['emit-declaration']
---*/

export class Color {
    r: int
    g: int
    b: int
    constructor(r: int, g: int, b: int) {
        this.r = r
        this.g = g
        this.b = b
    }
    toString(): string {
        return this.r + "," + this.g + "," + this.b
    }
}

export function mix(a: Color, b: Color): Color {
    return new Color((a.r + b.r) / 2, (a.g + b.g) / 2, (a.b + b.b) / 2)
}
