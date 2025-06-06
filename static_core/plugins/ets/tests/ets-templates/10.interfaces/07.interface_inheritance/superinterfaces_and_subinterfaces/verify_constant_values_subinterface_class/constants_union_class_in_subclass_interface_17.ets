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
desc: Verify whether the constant values UnionTypeClass_interface[] remain consistent in the subclass interface.
tags: [compile-only, negative]
---*/

class User {
  user_name: string;
  constructor() {
    this.user_name = "test_name";
  }
}

interface SomeInterface {
  interface_name: string;
}

class UnionTypeClass {
  public value: User[] | SomeInterface[];

  constructor(value: User[] | SomeInterface[]) {
    this.value = value;
  }
}

const obj1: SomeInterface = { interface_name: "Object1" };
const obj2: SomeInterface = { interface_name: "Object2" };
const temp_value: SomeInterface[] = [obj1, obj2];

interface ParentInterface {
  const CONSTANT_VALUE: UnionTypeClass;
}

interface SubInterface extends ParentInterface {}

class MyClass implements SubInterface {
  const CONSTANT_VALUE: UnionTypeClass = new UnionTypeClass(temp_value);
}

class SubClass extends MyClass {}

function main(): void {
  const sub = new SubClass();
}