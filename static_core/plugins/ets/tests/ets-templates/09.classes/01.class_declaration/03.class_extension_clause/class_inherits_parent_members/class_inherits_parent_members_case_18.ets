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
desc: Ensure that a subclass inherits the initial value of a Date type member variable from the parent class.
---*/

const date = new Date(2023, 10, 1);

class Base {
  protected value: Date;

  constructor() {
    this.value = date;
  }
}

class Derived extends Base {
  public value1: Date;
  constructor() {
    super();
    this.value1 = this.value;
  }
}

function main(): void {
  let instance = new Derived();
  assertEQ(instance.value1, date, "Subclass should inherit the initial value of the Date type member variable");
}