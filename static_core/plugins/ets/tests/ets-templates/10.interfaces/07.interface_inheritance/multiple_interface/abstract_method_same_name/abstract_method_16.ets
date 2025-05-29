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
desc: The method with the same name has the same type and number of parameters, but a method has a default implementation.
---*/

let result: string = "";

interface Parent1 {
  log(message: string): void;
}

interface Parent2 {
  log(message: string): void; 
}

interface Child extends Parent1, Parent2 {
  log(message: string): void; 
}

class ChildClass implements Child {
  log(message: string): void {
    result = `Logged: ${message}`;
  }
}

function main(): void {
  const child = new ChildClass();
  child.log("Hello");
  assertEQ(result, "Logged: Hello");
}