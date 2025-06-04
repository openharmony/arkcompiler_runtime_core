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
desc: Static methods need to be defined directly in the class rather than in the interface.
---*/

interface MyInterface {
  instanceMethod(): void;
}

class MyClass implements MyInterface {
  instanceMethod(): void {
    console.log("This is an instance method.");
  }

  // Define static methods directly in a class
  static staticMethod(): string {
    return "This is a static method.";
  }
}

function main(): void {
  const result = MyClass.staticMethod();
  assertEQ(result, "This is a static method.");
}