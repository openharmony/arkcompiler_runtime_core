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
desc: Library exporting public API surface that uses every primitive value type plus Any
tags: [not-a-test, compile-only]
name: test_lib
arktsconfig: arktsconfig-lib.json
es2panda_options: ['emit-metadata']
---*/

export function takeShort(x: short): short {
    return Int.toShort(x + 1)
}

export function takeLong(x: long): long {
    return x + 1
}

export function takeFloat(x: float): float {
    return x + 1.0f
}

export function takeChar(x: char): char {
    return x
}

export function takeAny(v: Any): string {
    return `${v}`
}

export function makeAny(): Any {
    return 42
}

export class PrimBag {
    public s: short = 0
    public l: long = 0
    public f: float = 0.0f
    public c: char = c'A'
    public a: Any = "hello"

    constructor(s: short, l: long, f: float, c: char, a: Any) {
        this.s = s
        this.l = l
        this.f = f
        this.c = c
        this.a = a
    }

    public summary(): string {
        return `${this.s}|${this.l}|${this.f}|${this.c}|${this.a}`
    }
}
