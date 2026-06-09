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

export class Base {}
export interface IBase {}

// --- @noninterop: removes the declaration entirely ---

/** @noninterop */
export class RemovedClass {}

/** @noninterop */
export interface RemovedInterface {}

/** @noninterop */
export enum RemovedEnum { A = 0 }

/** @noninterop */
export function removedFunc(): void {}

/** @noninterop */
export let removedVar: string = '';

/** @noninterop */
export type RemovedAlias = string;

// @noninterop on individual class members
export class ClassWithNoninteropMembers {
    keptMethod(): string { return ''; }
    /** @noninterop */
    removedMethod(): void {}
    keptProp: number = 0;
    /** @noninterop */
    removedProp: string = '';
}

// @noninterop on individual interface members
export interface IWithNoninteropMembers {
    keptMethod(): string;
    /** @noninterop */
    removedMethod(): void;
    keptProp: number;
    /** @noninterop */
    removedProp: string;
}

// @noninterop on getter/setter/constructor
export class ClassWithAccessors {
    get keptProp(): string { return ''; }
    /** @noninterop */
    get removedGetter(): string { return ''; }
    /** @noninterop */
    set removedSetter(v: string) {}
    /** @noninterop */
    constructor(x: number) {}
}

// @noninterop on individual enum members
export enum PartialEnum {
    KeptA = 0,
    /** @noninterop */
    RemovedB = 1,
    KeptC = 2
}

// @noninterop on call/construct/index signatures
export interface IWithSignatures {
    keptProp: string;
    /** @noninterop */
    (x: number): void;
    /** @noninterop */
    new(x: number): IWithSignatures;
    /** @noninterop */
    [key: string]: string;
}

// --- @interop any: replaces class/interface/enum with `type Name = Any;` ---

/** @interop any */
export class AnyClass {
    method(): void {}
}

/** @interop any */
export declare interface AnyInterface {
    prop: string;
}

/** @interop any */
export enum AnyEnum {
    A = 0,
    B = 1
}

// --- @interop break-extends: strips the extends heritage clause ---

/** @interop break-extends */
export class BreakExtendsClass extends Base {}

/** @interop break-extends */
export interface BreakExtendsInterface extends IBase {}

// --- @interop break-implements: strips the implements heritage clause ---

/** @interop break-implements */
export class BreakImplementsClass implements IBase {}

// --- @interop ret: changes the return type ---

/** @interop ret string */
export function retFunc(x: number): void {}

// --- @interop param: changes a parameter type ---

/** @interop param 0 string */
export function paramFunc(x: number): void {}

// --- class members using @interop ret and @interop param ---

export class Methods {
    /** @interop ret number */
    getValue(): string { return ''; }
    /** @interop param 1 string */
    process(a: number, b: boolean): void {}
    /** @interop param 0 string */
    constructor(x: number) {}
}

// --- interface method signatures using @interop ret and @interop param ---

export interface IMethodSigs {
    /** @interop ret number */
    getValue(): string;
    /** @interop param 1 string */
    process(a: number, b: boolean): void;
}
