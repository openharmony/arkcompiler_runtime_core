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
desc: Verify whether an interface can inherit from an interface modified with final.
tags: [compile-only, negative]
---*/

final interface BaseInterface {
  method1(): void;
}

interface SubInterface extends BaseInterface {
  method2(): void;
}

class MyClass implements SelfInheritingInterface {
  method1(): void {
    console.log("run result --> faild: expected output --> , actual output --> SyntaxError --> \n" +
                "Unexpected token interface, actual error --> ES2PANDA_FAIL, actual return code --> 1");
  }

  method2(): void {
    console.log("run result --> faild: expected output --> , actual output --> SyntaxError --> \n" +
                "Unexpected token interface, actual error --> ES2PANDA_FAIL, actual return code --> 1");
  }
}

function main(): void {
  const classA = new MyClass();
  classA.method1();
  classA.method2();
}