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
desc: Subclass overrides parent method and uses parent's getter/setter.
---*/

class Parent {
  private _greeting: string = "Hello from Parent";

  get greeting(): string {
    return this._greeting;
  }

  set greeting(value: string) {
    this._greeting = value;
  }

  greet(): string {
    return this.greeting;
  }
}

class Child extends Parent {
  greet(): string {
    return super.greet() + " and Hello from Child";
  }
}

function main(): void {
  let child = new Child();
  assertEQ(child.greet(), "Hello from Parent and Hello from Child");
}