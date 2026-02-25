/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

interface I0 {
    I0Method(a: string): string { return a; };
}

interface I1 {
    I1Method(a: double): double { return a; };
}

class Base {
    a: double = 1;
    constructor(a: double) {this.a = a;}
}

export class Derived extends Base implements I0, I1 {
    I0Method(a: string): string { return a; };
    I1Method(a: double): double { return a; };
    b: double = 2;
    constructor(a: double, b: double) {super(a); this.b = b;}
}

export class C1 {
    a: CC1 = new CC1;
    foo(a: CC2): CC3 {
        return new CC3;
    }
}
export interface I2 {
    a: CI1;
    foo(a: CI2): CI3;
}
export let p1: CP1;
export function foo1(a: CP2): CP3 {
    return new CP3;
}
export let p5: CP4 | CP5;
export function foo5(a: CP6 | CP7): CP8 | CP9 {
  return new CP8;
}
export let p6: IP1;
export function foo6(a: IP2): IP3 {
  return new CC7;
}
export let p8: NN1.C1;
export let p9: Promise<CP10>;
export let p10: Promise<IP5>;
export class C4<T extends CC4> {}
export class C5 extends CC5 implements IP4 {}


export class CC1 {}
export class CC2 {}
export class CC3 {}
class CC4 {}
class CC5 {}
class CC7 implements IP3{}
export class CP1 {}
export class CP2 {}
export class CP3 {}
export class CP4 {}
export class CP5 {}
export class CP6 {}
export class CP7 {}
export class CP8 {}
export class CP9 {}
export class CP10 {}
export class CI1 {}
export class CI2 {}
export class CI3 {}
export interface IP1 {}
export interface IP2 {}
export interface IP3 {}
interface IP4 {}
export interface IP5 {}
enum E1 {
  A
}
enum E2 {
  A
}
enum E3 {
  A
}
enum E4 {
  A
}
namespace NN1 {
    export class C1 {}
}


interface IN {}
class CN implements IN {}


namespace image {
    export interface PixelMap {}
}
class Want {}
export type ValueType = number | string | boolean | image.PixelMap | Want | ArrayBuffer | object | null | undefined;

export let p3: E5;
export function foo3(a: E6): E7 {
  return E7.A;
}
export let p12: Promise<E8>;
export enum E5 {
  A
}
export enum E6 {
  A
}
export enum E7 {
  A
}
export enum E8 {
  A
}

type BaseType = string;
type SubType<T> = BaseType | ((item: T, index: number) => void);
type PropertyType<T> = SubType<T> | ((item: T, index: number) => string);

type InterfacePropertyType = string;
type InterfaceFunctionParamType = number;
type InterfaceFunctionReturnType = boolean;
export interface InterfacePropertyInterface {}
export interface InterfaceFunctionParamInterface {}
export interface InterfaceFunctionReturnInterface {}
export class InterfacePropertyClass {}
export class InterfaceFunctionParamClass {}
export class InterfaceFunctionReturnClass {}
export interface BaseInterface {}
export interface SubInterface extends BaseInterface {}
export interface PropertyInterface extends SubInterface {
  t: InterfacePropertyType;
  i: InterfacePropertyInterface;
  c: InterfacePropertyClass;
  fti(p: InterfaceFunctionParamType): InterfaceFunctionReturnInterface;
  fic(p: InterfaceFunctionParamInterface): InterfaceFunctionReturnClass;
  fct(p: InterfaceFunctionParamClass): InterfaceFunctionReturnType;
}
class PropertyInterfaceImplementation implements PropertyInterface {
  t: InterfacePropertyType = "default";
  i: InterfacePropertyInterface = {};
  c: InterfacePropertyClass = {};
  fti(p: InterfaceFunctionParamType): InterfaceFunctionReturnInterface { return {}; }
  fic(p: InterfaceFunctionParamInterface): InterfaceFunctionReturnClass { return {}; }
  fct(p: InterfaceFunctionParamClass): InterfaceFunctionReturnType { return true; }
}

type ClassPropertyType = string;
type ClassFunctionParamType = number;
type ClassFunctionReturnType = boolean;
export interface ClassPropertyInteface {}
export interface ClassFunctionParamInterface {}
export interface ClassFunctionReturnInterface {}
export class ClassPropertyClass {}
export class ClassFunctionParamClass {}
export class ClassFunctionReturnClass {}
export interface ImplementedByBase1 {}
export interface ImplementedByBase2 {}
export interface ImplementedBySub {}
export interface ImplementedByProperty {}
export class BaseClass implements ImplementedByBase1, ImplementedByBase2 {}
export class SubClass extends BaseClass implements ImplementedBySub {}
export class PropertyClass extends SubClass implements ImplementedByProperty {
  t: ClassPropertyType = "default";
  i: ClassPropertyInteface = {};
  c: ClassPropertyClass = {};
  fti(p: ClassFunctionParamType): ClassFunctionReturnInterface { return {}; }
  fic(p: ClassFunctionParamInterface): ClassFunctionReturnClass { return {}; }
  fct(p: ClassFunctionParamClass): ClassFunctionReturnType { return true; }
}

export class ExportedClass {
  t: PropertyType<boolean> = "default";
  i: PropertyInterface = new PropertyInterfaceImplementation();
  c: PropertyClass = new PropertyClass();
}
