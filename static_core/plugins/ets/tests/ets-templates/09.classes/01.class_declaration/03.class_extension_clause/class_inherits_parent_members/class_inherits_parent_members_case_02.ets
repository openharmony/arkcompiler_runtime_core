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
desc: Ensure that a subclass inherits the initial value of a number[] type member variable from the parent class.
---*/

class Base {
  protected value: number[];

  constructor() {
    this.value = [1, 2, 3];
  }
}

class Derived extends Base {
  public value1: number[];
  constructor() {
    super();
    this.value1 = this.value;
  }
}

function main(): void {
  let instance = new Derived();
  assertEQ(instance.value1[0], 1, "Subclass should inherit the initial value of the number[] type member variable");
  assertEQ(instance.value1[1], 2, "Subclass should inherit the initial value of the number[] type member variable");
  assertEQ(instance.value1[2], 3, "Subclass should inherit the initial value of the number[] type member variable");
}