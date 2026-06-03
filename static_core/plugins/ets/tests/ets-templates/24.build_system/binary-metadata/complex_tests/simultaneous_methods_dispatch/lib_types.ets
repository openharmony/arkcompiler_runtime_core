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
desc: Package @panda/types - base types for method dispatch testing (simultaneous with lib_impls)
tags: [not-a-test, compile-only]
name: lib_types
arktsconfig: arktsconfig-types.json
es2panda_options: ['emit-metadata', 'simultaneous']
---*/

// --- 15.11: Base class for override and dynamic dispatch testing ---
export class Animal {
    private _name: string
    constructor(name: string) {
        this._name = name
    }
    speak(): string {
        return this._name + ":generic"
    }
    getName(): string {
        return this._name
    }
}

// --- 15.11: Abstract class for abstract override testing ---
export abstract class Renderer {
    abstract render(content: string): string
    prefix(): string {
        return ">>>"
    }
}

// --- 15.11.10: Interface with default method ---
export interface Labeled {
    getLabel(): string
    fullLabel(): string {
        return "[" + this.getLabel() + "]"
    }
}

// --- 15.12: Generic class for type erasure testing ---
export class Holder<T> {
    private _value: T
    constructor(value: T) {
        this._value = value
    }
    get value(): T {
        return this._value
    }
}

// --- 15.11.10: Static methods (no dynamic dispatch) ---
export class Counter {
    private static _count: int = 0
    static increment(): void {
        Counter._count = Counter._count + 1
    }
    static getCount(): int {
        return Counter._count
    }
    static reset(): void {
        Counter._count = 0
    }
}

// --- 15.10: Class with overloaded methods ---
export class TextFormatter {
    format(val: number): string {
        return "fmt_n:" + val.toString()
    }
    format(val: string): string {
        return "fmt_s:" + val
    }
}

// --- 15.9: Class with multiple methods (some inherited, some overridden) ---
export class Vehicle {
    private _brand: string
    constructor(brand: string) {
        this._brand = brand
    }
    getBrand(): string {
        return this._brand
    }
    describe(): string {
        return "vehicle:" + this._brand
    }
    honk(): string {
        return "beep"
    }
}

