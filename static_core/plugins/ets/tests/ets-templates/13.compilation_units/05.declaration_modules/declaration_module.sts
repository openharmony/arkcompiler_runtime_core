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
desc: Sample from spec, Namespaces with identical namespace names in a single compilation unit merge their exported declarations into a single namespace.
tags: [compile-only, not-a-test]
---*/

export declare namespace NS_A{
    export function foo(): void
}

export namespace NS_B{
    export declare function foo(): void
}

export declare class A {}

export declare class B {
    public a: A // the field is exported but its type is not
}

declare class C {}

declare class DD {
    public static fdd(): void
}

declare class EE {
    public fee(): string
}

export {DD, EE as ZZ}

export declare class Person {
  constructor(name: string, age: number);
  getName(): string;
  getAge(): number;
  static createDefault(): Person;
}

export declare function process_field(p: A): void

export type Matrix = number[][]
export type Handler = (s: string, no: number) => string

export type Predicate<T> = (x: T) => Boolean
export type NullableNumber = Number | null
export type Dictionary = Map<string, string>
export type MapOfString<T> = Map<T, string>
export type AAA<T> = Array<AAA<T>> // ok, AAA<T> is used as a type argument
export type AAA2<T> = string | Array<AAA2<T>> // ok
