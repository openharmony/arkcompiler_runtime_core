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
desc: Subclasses accessing the initialization interface[] of parent class members.
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

let check_value: SomeInterface | undefined;

class Sub extends Base {
  constructor() {
    super();
  }

  public checkAccess(): void {
    check_value = this.value[0];
  }
}

function main(): void {
  const instanceB = new Sub();
  instanceB.checkAccess();
  assertEQ(check_value!.name, "Object1");
}