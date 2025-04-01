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
desc: The subclass modifies the value of its own member variable that is directly initialized in the constructor. The modifier is private and the type is number.
---*/

class Parent {
  private constructorPublicVar: number;
}

class Child extends Parent {
  private childField1 : number = 200;

  constructor() {
    this.childField1 = 100;
  }

  public getProtectedValueFromChild(): number {
    return this.childField1;
  }
}

function main(): void {
  let child = new Child();
  let x: number = child.getProtectedValueFromChild();
  assertEQ(x, 100);
}