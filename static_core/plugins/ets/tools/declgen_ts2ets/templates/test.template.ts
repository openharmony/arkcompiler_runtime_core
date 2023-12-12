/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

export namespace N {
  export type Generic<T> = T;
  export type Undef = undefined;
  export type Any = any;
  export type Union = number | Any | string;
  export type UnionWithNull = number | null | Any | string;
  export type Nullable1 = null | string;
  export type Nullable2 = string | null;
  export type StringLitType = "ease-in" | "ease-out" | "ease-in-out";

  // as field in class
  export class UserClass1 {
    static field1: TYPE;
  }

  // as variable
  export let GLOB: TYPE;

  // as argument and return value
  export function func(arg: TYPE): TYPE {
    return arg;
  }

  export class UserClass2 {
    func2(arg: TYPE, arg2: unknown): TYPE {
      return arg;
    }
  }

  export class UserClass3 {
    // as type argument
    field3: Generic<TYPE>;
    // as array type
    field4: TYPE[];
    // as intersection type
    field5: TYPE[] & Generic<TYPE> & TYPE & undefined;
    // as union type
    field6: TYPE[] | Generic<TYPE> | TYPE | undefined;
  }

  // never type test
  export function Never(): never {
    throw new Error("");
  }

  // test tuple
  export function fooTuple(): [string, number] {
    return ["1", 1];
  }

  // test object literal
  export function fooObject(): { res: string } {
    return { res: "34" };
  }

  // test # private conversion
  export class UserClass4 {
    #field1: number = 1;
    #foo(): void {}
    private field2: number = 1;
    private baz(): void {}
    readonly field3: number = 1;
  }
}

function freeFunction(): number {
  return 44;
}

export function freeExportFunction(): number {
  return 4;
}

export function optional1(a: number, arg?: string): void {}

export function optional2(a?: number, arg?: string): void {}

export function optional3(a: number | null, arg?: string): void {}

export function optional4(a?: number | string, arg?: string): void {}

export function optional5(a?: boolean, arg?: string): void {}

export function optional6(a?: N.UserClass1, arg?: string): void {}

export interface Optional1 {
  a?: string;
  b?: number;
  c?: boolean;
}

export class Optional2 {
  a?: string = "";
  b?: number = 1;
  c?: boolean = undefined;
}

export interface D {
  foo(): this;
  foo2(): D;
}

export class Box {
  Set(value: string): this {
    return this;
  }
}

export type s = { ss: number; kk: string };
export type S = s["ss"];
export type ss = { ff: s; 2: s };

export type X<T> = T extends number ? T : never;
export type XX<T> = T extends number ? T : number;
export type Y<T> = T extends number ? T : never;
export type YY<T> = T extends Array<infer Item> ? Item : never;

export class Q {
  static d: number = 1;
  static d2: typeof Q.d;

  static j: unknown;
  static j2: typeof Q.j;
}

export interface NewCtor {
  new (arg: string): NewCtor;
  foo(): this;
  foo2(): D;
}

export type OptionsFlags<Type> = {
  [Property in keyof Type]: boolean;
};

export type MappedTypeWithNewProperties<Type> = {
  [Properties in keyof Type as number]: Type[Properties];
};

export function calculateMyReturnType() {
  return 1;
}

export class Point {
  x: number = 1;
  y: number = 2;
}

export type PointKeys = keyof Point;

export type SomeConstructor = {
  new (s: string): number;
};

export interface DescribableFunction {
  description: string;
  (someArg: number): string; // call signature
}

export class Person {
  constructor(name: string, age: number) {}
}
export type PersonCtor = new (name: string, age: number) => Person;

export function rdonly1(arr: readonly string[]) {
  arr.slice();
}

export type A = Awaited<Promise<string>>;
export type B = Awaited<Promise<Promise<number>>>;
export type C = Awaited<boolean | Promise<number>>;
export type Part = Partial<Point>;
export type Req = Required<Part>;
export type Rdonly = Readonly<Point>;
export type Rec = Record<
  "miffy" | "boris" | "mordred",
  {
    age: number;
    breed: string;
  }
>;
export type TodoPreview = Pick<
  {
    title: string;
    description: string;
    completed: boolean;
  },
  "title" | "completed"
>;
export type TodoInfo = Omit<
  {
    title: string;
    description: string;
    completed: boolean;
    createdAt: number;
  },
  "completed" | "createdAt"
>;
export type T3 = Exclude<
  { kind: "circle"; radius: number } | { kind: "square"; x: number },
  { kind: "circle" }
>;
export type T2 = Extract<
  { kind: "circle"; radius: number } | { kind: "square"; x: number },
  { kind: "circle" }
>;
export type T0 = NonNullable<string | number | undefined>;
export type T1 = NonNullable<string[] | null | undefined>;
export type TT0 = Parameters<() => string>;
export type TT1 = Parameters<(s: string) => void>;
export type TT2 = Parameters<<T>(arg: T) => T>;
export type TT4 = Parameters<any>;
export type TT5 = Parameters<never>;
export type TC0 = ConstructorParameters<ErrorConstructor>;
export type TC1 = ConstructorParameters<FunctionConstructor>;
export type TC2 = ConstructorParameters<RegExpConstructor>;
export type TC4 = ConstructorParameters<any>;
export type TR0 = ReturnType<() => string>;
export type TR1 = ReturnType<(s: string) => void>;
export type TR2 = ReturnType<<T>() => T>;
export type TR3 = ReturnType<<T extends U, U extends number[]>() => T>;
export type TR5 = ReturnType<any>;
export type TR6 = ReturnType<never>;
export type TI1 = InstanceType<any>;
export type TI2 = InstanceType<never>;
export type TTP = ThisParameterType<Array<Number>>;
export type TOP = OmitThisParameter<Array<Number>>;
export type TTT = ThisType<Array<Number>>;
import m = require("mod");
export interface F extends m {}
import { default as d } from "def";
export interface DD extends d {}
import { default as dd, KK, HH as H, default as ddd } from "def";
export interface DD extends dd {}
export interface DD extends KK {}
export interface DD extends H {}
export interface DD extends ddd {}
export namespace KK {
  import { default as d } from "def";
  export interface DD extends d {}
  import { default as dd, KK, HH as H, default as ddd } from "def";
  export interface DD extends dd {}
  export interface DD extends KK {}
  export interface DD extends H {}
  export interface DD extends ddd {}
}
import { LKL } from "def.js";
export interface DD extends LKL {}
import { LL } from "def.json" assert { type: "json" };
export interface DD extends LL {}
import { default as LLL } from "def.json" assert { type: "json" };
export interface DD extends LLL {}
import type { TType } from "def";
export interface DD extends TType {}
export class KeyA {
  [key: string]: boolean;
}
export interface KeyB {
  [key: string]: boolean;
}
export type KeyC = {
  [key: string]: boolean;
};
