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
desc: When multiple parent interfaces have abstract methods with the same name, the processing of subclass interfaces.
---*/

let result: string = "";

interface ParentInterface1 {
  method(): void;
}

interface ParentInterface2 {
  method(): void;
}

interface ChildInterface extends ParentInterface1, ParentInterface2 {
  method(): void;
}

class ChildClass implements ChildInterface {
  method(): void {
    console.log("Implemented");
    result = "Implemented";
  }
}

function main(): void {
  const childInstance = new ChildClass();
  childInstance.method();
  assertEQ(result, "Implemented")
}