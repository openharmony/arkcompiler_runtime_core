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
desc: The child class calls the parent class method, and the getter overridden by the child class is used in the parent class method.
---*/

class Parent {
  private _message: string = "Hello from Parent!";

  get message(): string {
    return this._message;
  }

  public greet(): string {
    return this.message;
  }
}

class Child extends Parent {
  private _message: string = "Hello from Child!";

  get message(): string {
    return this._message;
  }

  public callParentGreet(): string {
    return this.greet();
  }
}

function main(): void {
  const child = new Child();
  assertEQ(child.callParentGreet(), "Hello from Child!");
}