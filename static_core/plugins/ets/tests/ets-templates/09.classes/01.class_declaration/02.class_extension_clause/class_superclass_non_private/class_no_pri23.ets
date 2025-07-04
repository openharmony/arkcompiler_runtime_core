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
desc: The child class calls the parent class method, and the parent class method uses the setter overridden by the child class.
tags: [compile-only, negative]
---*/

class Parent {
  private _message: string = "Hello from Parent!";

  set message(value: string) {
    this._message = value;
  }

  public greet(): string {
    return this._message;
  }
}

class Child extends Parent {
  set message(value: string) {
    this._message = "Modified: " + value;
  }

  public callParentGreet(): string {
    this.message = "Hello from Child!";
    return this.greet();
  }
}

function main(): void {
  const child = new Child();
  assertNotEqual(child.callParentGreet(), "Modified: Hello from Child!");
}