/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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


export interface Calculator {
  add(a: number, b: number): number;
}

export interface Calculator {
  subtract(a: number, b: number): number;
}

export interface I {
  foo(a: number):void;
}

export interface I {
  foo(a?: number):void;
}

export interface I {
  setAge(age: number): void;
}

export namespace A {
  export interface I {
      clone(arg0: string): string;
  }
  export interface I {
      name: string
  }
}

