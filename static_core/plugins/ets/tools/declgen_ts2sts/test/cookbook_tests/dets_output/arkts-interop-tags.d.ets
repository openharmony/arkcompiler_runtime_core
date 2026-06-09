'use static'
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
export declare type AnyClass = Any;
export declare type AnyInterface = Any;
export declare type AnyEnum = Any;
export declare function retFunc(x: number): string;
export declare function paramFunc(x: string): void;
export declare enum PartialEnum {
    KeptA = 0,
    KeptC = 2
}
export declare class Base {
}
export declare class ClassWithNoninteropMembers {
    keptMethod(): string;
    keptProp: number;
}
export declare class ClassWithAccessors {
    get keptProp(): string;
}
export declare class BreakExtendsClass {
}
export declare class BreakImplementsClass {
}
export declare class Methods {
    getValue(): number;
    process(a: number, b: string): void;
    constructor(x: string);
}
export declare interface IBase {
}
export declare interface IWithNoninteropMembers {
    keptMethod(): string;
    keptProp: number;
}
export declare interface IWithSignatures {
    keptProp: string;
}
export declare interface BreakExtendsInterface {
}
export declare interface IMethodSigs {
    getValue(): number;
    process(a: number, b: string): void;
}
