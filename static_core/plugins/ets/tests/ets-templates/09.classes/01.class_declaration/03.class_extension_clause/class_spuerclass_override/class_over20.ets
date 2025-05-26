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
desc: The child class overrides the parent class method and uses the chained invocation of the parent class.
---*/

class Parent {
  value: string = "";

  greet(): this {
    this.value += "Hello from Parent";
    return this;
  }

  getValue(): string {
    return this.value;
  }
}

class Child extends Parent {
  greet(): this {
    super.greet();
    this.value += " and Hello from Child";
    return this;
  }
}

function main(): void {
  let child = new Child();
  assertEQ(child.greet().getValue(), "Hello from Parent and Hello from Child");
}