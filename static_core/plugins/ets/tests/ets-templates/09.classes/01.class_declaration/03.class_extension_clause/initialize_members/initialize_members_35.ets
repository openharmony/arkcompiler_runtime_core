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
desc: The subclass modifies the value of the member variable directly initialized by the parent class in the constructor function. The modifier is public and the type is number.
---*/

class Parent {
  public constructorPublicVar: number = 400;
}

class Child extends Parent {
  constructor() {
    this.constructorPublicVar = 200;
  }
}

function main(): void {
  let child = new Child();
  let x: number = child.constructorPublicVar;
  assertEQ(x, 200);
}