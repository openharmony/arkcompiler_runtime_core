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
desc: A subclass calls multiple non - private methods of its superclass.
---*/

class Parent {
  public greet(): string {
    return "Hello";
  }

  public farewell(): string {
    return "Word";
  }
}

class Child extends Parent {
  public callParentMethods(): string {
    return `${this.greet()} ${this.farewell()}`;
  }
}

function main(): void {
  const child = new Child();
  assertEQ(child.callParentMethods(), "Hello Word");
}