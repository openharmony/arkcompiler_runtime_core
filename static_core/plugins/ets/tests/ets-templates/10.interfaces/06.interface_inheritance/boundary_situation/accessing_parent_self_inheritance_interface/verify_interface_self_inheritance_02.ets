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
desc: Test the compiler's reaction when an interface inherits from itself.
tags: [compile-only, negative]
---*/

interface InterfaceA extends InterfaceB {
  method1(): void;
}

interface InterfaceB extends InterfaceA {
  method2(): void;
}

class MyClass implements InterfaceA {
  method1(): void {
    console.log("run result --> no pass: expected output --> , actual output --> ,\n" +
                "actual error --> SEGFAULT_FAIL, actual return code --> -11");
  }

  method2(): void {
    console.log("run result --> no pass: expected output --> , actual output --> ,\n" +
                "actual error --> SEGFAULT_FAIL, actual return code --> -11");
  }
}

function main(): void {
  const classA = new MyClass();
  classA.method1();
  classA.method2();
}