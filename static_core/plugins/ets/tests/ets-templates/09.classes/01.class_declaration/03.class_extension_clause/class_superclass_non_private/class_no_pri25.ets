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
desc: The child class calls the parent class method, which uses the static properties overridden by the child class.
---*/

class Parent {
  static staticMessage: string = "Static Message from Parent";
  public greet(): string {
    return Parent.staticMessage;
  }
}

class Child extends Parent {
  static staticMessage: string = "Static Message from Child";
  public callParentGreet(): string {
    return this.greet();
  }
}

function main(): void {
  const child = new Child();
  assertEQ(child.callParentGreet(), "Static Message from Parent");
}