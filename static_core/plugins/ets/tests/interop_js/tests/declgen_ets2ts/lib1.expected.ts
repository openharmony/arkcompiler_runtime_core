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

declare const exports: any;

import { Lib2Class, Lib2Class2, DoubleAlias } from ".//lib2";

export declare function _$init$_(): void;
exports._$init$_ = (globalThis as any).Panda.getFunction('LETSGLOBAL;', '_$init$_');
exports._$init$_();

export declare function SumDouble(a: number, b: number): number;
exports.SumDouble = (globalThis as any).Panda.getFunction('LETSGLOBAL;', 'SumDouble');

export declare function SumString(a: string, b: string): string;
exports.SumString = (globalThis as any).Panda.getFunction('LETSGLOBAL;', 'SumString');

export declare function Identity<T>(x: T): T;
exports.Identity = (globalThis as any).Panda.getFunction('LETSGLOBAL;', 'Identity');

export declare function ForEach<T extends Object>(a: T[], cb: (e: T, idx: number) => void): void;
exports.ForEach = (globalThis as any).Panda.getFunction('LETSGLOBAL;', 'ForEach');

export declare function ReplaceFirst(arr: Simple[], e: Simple): Simple[];
exports.ReplaceFirst = (globalThis as any).Panda.getFunction('LETSGLOBAL;', 'ReplaceFirst');

export declare class Fields {
    public readonlyBoolean_: boolean;
    public static staticFloat_: number;
    public double_: number;
    public doubleAlias_: number;
    public int_: number;
    public string_: string;
    public nullableString_: string | null;
    public object_: Object;
    public rangeError_: RangeError;
    public uint8Array_: Uint8Array;
    public uint16Array_: Uint16Array;
    public uint32Array_: Uint32Array;
    constructor();
};
exports.Fields = (globalThis as any).Panda.getClass('LFields;');

export declare class Methods {
    public IsTrue(a: boolean): boolean;
    public SumString(a: string, b: string): string;
    public static StaticSumDouble(a: number, b: number): number;
    public SumIntArray(a: number[]): number;
    public OptionalString(a?: string | null): string | null;
    constructor();
};
exports.Methods = (globalThis as any).Panda.getClass('LMethods;');

export enum ETSEnum {
    Val0 = 0,
    Val1 = 1,
    Val2 = 2,
}

export interface I0 {
    I0Method(a: string): string;
}

export interface I1 {
    I1Method(a: number): number;
}

export declare class Base {
    public a: number;
    constructor();
};
exports.Base = (globalThis as any).Panda.getClass('LBase;');

export declare class Derived extends Base implements I0, I1 {
    public I0Method(a: string): string;
    public I1Method(a: number): number;
    public b: number;
    constructor();
};
exports.Derived = (globalThis as any).Panda.getClass('LDerived;');

export interface Interface0 {
    a(): void;
    b(): void;
    d(): void;
}

export declare class Simple {
    public constructor(x: number);
    public val: number;
};
exports.Simple = (globalThis as any).Panda.getClass('LSimple;');

export declare class External {
    public constructor();
    public o1: Simple;
    public o2: Lib2Class;
    public o3: Lib2Class2;
};
exports.External = (globalThis as any).Panda.getClass('LExternal;');

export declare class GenericClass<T extends Object> {
    public identity(x: T): T;
    constructor();
};
exports.GenericClass = (globalThis as any).Panda.getClass('LGenericClass;');

export interface IGeneric0<T> {
    I0Method(a: T): T;
}

export interface IGeneric1<T> {
    I1Method(a: T): T;
}

export declare class BaseGeneric<T, U> {
    public c: T;
    public d: U;
    constructor();
};
exports.BaseGeneric = (globalThis as any).Panda.getClass('LBaseGeneric;');

export declare class DynObjWrapper {
    public dynObj_: any;
    constructor();
};
exports.DynObjWrapper = (globalThis as any).Panda.getClass('LDynObjWrapper;');

export declare class PromiseWrapper {
    public promise_: Promise<string>;
    constructor();
    public static lambda$invoke$0: any;
};
exports.PromiseWrapper = (globalThis as any).Panda.getClass('LPromiseWrapper;');

