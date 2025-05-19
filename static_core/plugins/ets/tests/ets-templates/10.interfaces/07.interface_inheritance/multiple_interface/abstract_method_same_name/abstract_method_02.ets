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
desc: Method signature of the same name is incompatible.
tags: [compile-only, negative]
---*/

let resultString: string = "";
let resultNumber: string = "";

interface Parent1 {
  greet(message: string): void;
}

interface Parent2 {
  greet(message: number): void;
}

interface Child extends Parent1, Parent2 {}

class ChildClass implements Child {
  greet(message: string | number): void {
    if (typeof message === "string") {
      resultString = `String Greeting: ${message}`;
    } else {
      resultNumber = `Number Greeting: ${message}`;
    }
  }
}

function main(): void {
  const child = new ChildClass();
}