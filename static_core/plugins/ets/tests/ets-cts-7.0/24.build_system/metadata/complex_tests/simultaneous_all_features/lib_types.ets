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
desc: Package @panda/types - type definitions (simultaneous with lib_impls)
tags: [not-a-test, compile-only]
name: lib_types
arktsconfig: arktsconfig-types.json
es2panda_options: ['emit-declaration', 'simultaneous']
---*/

// --- Enums ---
export enum Color { Red, Green, Blue }

export enum Status {
    Active = 1,
    Inactive = 0
}

// --- Computed enum values (compiler evaluates expressions at compile-time) ---
export enum Priority {
    Low = 10,
    Medium = Low + 10,
    High = Medium * 2
}

// --- Type Aliases ---
export type NumAlias = number
export type StringOrNull = string | null
export type Callback<T> = (value: T) => string
export type Mapper<T, U> = (input: T) => U

// --- Union type alias (non-nullable) ---
export type StringOrNum = string | number

// --- Ambient type alias declaration ---
export declare type Score = int

// --- Interfaces ---
export interface Printable {
    display(): string
}

export interface Named {
    getName(): string
}

export interface Describable extends Printable, Named {
    describe(): string
    summary(): string {
        return this.display() + "/" + this.getName()
    }
}

// --- Generic class ---
export class GenBox<T> {
    private _value: T
    constructor(value: T) {
        this._value = value
    }
    get value(): T {
        return this._value
    }
    set value(v: T) {
        this._value = v
    }
    map<U>(fn: (v: T) => U): GenBox<U> {
        return new GenBox<U>(fn(this._value))
    }
}

// --- Readonly fields ---
export class Config {
    readonly id: int
    readonly label: string
    constructor(id: int, label: string) {
        this.id = id
        this.label = label
    }
}

// --- Abstract class ---
export abstract class Shape {
    abstract area(): number
    perimeter(): number {
        return 0.0
    }
}

// --- Enum factory (co-located with enum to avoid cross-module type mismatch) ---
export function colorName(c: Color): string {
    if (c == Color.Red) return "red"
    if (c == Color.Green) return "green"
    return "blue"
}

// --- Config factory ---
export function makeConfig(id: int): Config {
    return new Config(id, "cfg-" + id.toString())
}

// --- Class with static fields and methods (compiler generates class-level init) ---
export class Registry {
    private static _count: int = 0
    private _name: string
    constructor(name: string) {
        Registry._count = Registry._count + 1
        this._name = name
    }
    getName(): string {
        return this._name
    }
    static getCount(): int {
        return Registry._count
    }
    static reset(): void {
        Registry._count = 0
    }
}

// --- Computed enum factory (co-located with enum) ---
export function priorityLabel(p: Priority): string {
    if (p == Priority.Low) return "low"
    if (p == Priority.Medium) return "medium"
    return "high"
}

// --- Recursive type alias (linked list, recursion through type alias) ---
export type IntList = IntNode | null

export class IntNode {
    head: int
    tail: IntList
    constructor(head: int, tail: IntList) {
        this.head = head
        this.tail = tail
    }
}

// --- Interfaces for utility type testing (Partial, Required, Readonly) ---
export interface Issue {
    title: string
    description: string
}

export interface OptIssue {
    title?: string
    description?: string
}

// --- Function type alias for ReturnType testing ---
export type StringProducer = () => string

// --- Container class with Object field ---
export class Container {
    label: string
    content: Object
    constructor(label: string, content: Object) {
        this.label = label
        this.content = content
    }
}

// --- Class with nullable field ---
export class Wrapper {
    content: string | null
    constructor(content: string | null) {
        this.content = content
    }
    getOrDefault(def: string): string {
        return this.content ?? def
    }
}
