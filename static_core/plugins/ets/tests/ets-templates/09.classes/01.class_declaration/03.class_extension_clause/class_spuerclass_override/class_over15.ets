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
desc: The child class overrides the parent class method and uses the static properties of the parent class.
---*/

class Parent {
  static staticGreeting: string = "Hello from Parent (static)";

  greet(): string {
    return Parent.staticGreeting;
  }
}

class Child extends Parent {
  greet(): string {
    return super.greet() + " and Hello from Child";
  }
}

function main(): void {
  let child = new Child();
  assertEQ(child.greet(), "Hello from Parent (static) and Hello from Child");
}