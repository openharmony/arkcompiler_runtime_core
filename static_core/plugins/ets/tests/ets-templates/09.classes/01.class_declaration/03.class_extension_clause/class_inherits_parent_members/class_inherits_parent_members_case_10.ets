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
desc: Ensure that a subclass inherits the initial value of a SomeInterface[] type member variable from the parent class.
---*/

interface SomeInterface {
  name: string;
}

const obj1: SomeInterface = { name: "Object1" };
const obj2: SomeInterface = { name: "Object2" };

class Base {
  protected value: SomeInterface[];

  constructor() {
    this.value = [obj1, obj2];
  }
}

class Derived extends Base {
  public value1: SomeInterface[];
  constructor() {
    super();
    this.value1 = this.value;
  }
}

function main(): void {
  let instance = new Derived();
  assertEQ(instance.value1[0].name, "Object1", "Subclass should inherit the initial value of the SomeInterface[] type member variable");
  assertEQ(instance.value1[1].name, "Object2", "Subclass should inherit the initial value of the SomeInterface[] type member variable");
}