/**
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

export interface Iface {
  n: number
  s() : string
}

export interface Iface2 {
  foo1() : void
}

export interface Iface3 {
  foo2(...nums: number[]) : void
}

export @interface Position {}

export function foo3(a : Array<string>) {
  console.log(a[0])
}
export class C {
  static foo3(a : Array<string>) { console.log(a[0]) }
  bar(a : Array<string>) { console.log(a[0]) }
}
export interface Iface4 {
  foo3(a : Array<string>) : void
}

export interface I {
  s: string
}

export interface Iface5 {
  foo5() : void
}
export function foo6(iface5 : Iface5) {
  console.log(iface5 instanceof Iface5) // true
}
