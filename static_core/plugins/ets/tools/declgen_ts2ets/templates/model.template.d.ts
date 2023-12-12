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

export declare namespace N {
    type Generic<T> = T;
    type Undef = JSValue;
    type Any = JSValue;
    type Union = number | Any | string;
    type UnionWithNull = number | null | Any | string;
    type Nullable1 = null | string;
    type Nullable2 = string | null;
    type StringLitType = "ease-in" | "ease-out" | "ease-in-out";
    class UserClass1 {
        static field1: TYPE;
    }
    let GLOB: TYPE;
    function func(arg: TYPE): TYPE;
    class UserClass2 {
        func2(arg: TYPE, arg2: JSValue): TYPE;
    }
    class UserClass3 {
        field3: Generic<TYPE>;
        field4: TYPE[];
        field5: JSValue;
        field6: TYPE[] | Generic<TYPE> | TYPE | JSValue;
    }
    function Never(): never;
    function fooTuple(): JSValue[];
    function fooObject(): JSValue;
    class UserClass4 {
        // private;
        private field2;
        private baz;
        readonly field3: number;
    }
}
export declare function freeExportFunction(): number;
export declare function optional1(a: number, arg?: string): void;
export declare function optional2(a: number | null, arg?: string): void;
export declare function optional3(a: number | null, arg?: string): void;
export declare function optional4(a?: number | string, arg?: string): void;
export declare function optional5(a: boolean | null, arg?: string): void;
export declare function optional6(a?: N.UserClass1, arg?: string): void;
export interface Optional1 {
    a?: string;
    b: number | null;
    c: boolean | null;
}
export declare class Optional2 {
    a?: string;
    b: number | null;
    c: boolean | null;
}
export interface D {
    foo(): D;
    foo2(): D;
}
export declare class Box {
    Set(value: string): Box;
}
export type s = JSValue;
export type S = number;
export type ss = JSValue;
export type X<T> = JSValue;
export type XX<T> = JSValue;
export type Y<T> = JSValue;
export type YY<T> = JSValue;
export declare class Q {
    static d: number;
    static d2: number;
    static j: JSValue;
    static j2: JSValue;
}
export type NewCtor = JSValue;
export type OptionsFlags<Type> = JSValue;
export type MappedTypeWithNewProperties<Type> = JSValue;
export declare function calculateMyReturnType(): number;
export declare class Point {
    x: number;
    y: number;
}
export type PointKeys = JSValue;
export type SomeConstructor = JSValue;
export type DescribableFunction = JSValue;
export declare class Person {
    constructor(name: string, age: number);
}
export type PersonCtor = JSValue;
export declare function rdonly1(arr: string[]): void;
export type A = JSValue;
export type B = JSValue;
export type C = JSValue;
export type Part = JSValue;
export type Req = JSValue;
export type Rdonly = JSValue;
export type Rec = JSValue;
export type TodoPreview = JSValue;
export type TodoInfo = JSValue;
export type T3 = JSValue;
export type T2 = JSValue;
export type T0 = JSValue;
export type T1 = JSValue;
export type TT0 = JSValue;
export type TT1 = JSValue;
export type TT2 = JSValue;
export type TT4 = JSValue;
export type TT5 = JSValue;
export type TC0 = JSValue;
export type TC1 = JSValue;
export type TC2 = JSValue;
export type TC4 = JSValue;
export type TR0 = JSValue;
export type TR1 = JSValue;
export type TR2 = JSValue;
export type TR3 = JSValue;
export type TR5 = JSValue;
export type TR6 = JSValue;
export type TI1 = JSValue;
export type TI2 = JSValue;
export type TTP = JSValue;
export type TOP = JSValue;
export type TTT = JSValue;
import * as m from "mod";
export interface F extends m {
}
import d from "def";
export interface DD extends d {
}
import { KK, HH as H } from "def";
import dd from "def";
import ddd from "def";
export interface DD extends dd {
}
export interface DD extends KK {
}
export interface DD extends H {
}
export interface DD extends ddd {
}
export declare namespace KK {
    import d from "def";
    interface DD extends d {
    }
    import { KK, HH as H } from "def";
    import dd from "def";
    import ddd from "def";
    interface DD extends dd {
    }
    interface DD extends KK {
    }
    interface DD extends H {
    }
    interface DD extends ddd {
    }
}
import { LKL } from "def";
export interface DD extends LKL {
}
import { LL } from "def.json";
export interface DD extends LL {
}
import LLL from "def.json";
export interface DD extends LLL {
}
import { TType } from "def";
export interface DD extends TType {
}
export declare type KeyA = JSValue;
export type KeyB = JSValue;
export type KeyC = JSValue;
