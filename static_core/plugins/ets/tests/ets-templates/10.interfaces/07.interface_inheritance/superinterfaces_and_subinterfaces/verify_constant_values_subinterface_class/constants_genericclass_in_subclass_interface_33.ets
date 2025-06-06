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
desc: Verify whether the constant values genericclass<class> remain consistent in the subclass interface.
tags: [compile-only, negative]
---*/

class GenericClass<T> {
  public value: T;

  constructor(value: T) {
    this.value = value;
  }
}

class User {
  name: string;
  constructor() {
    this.name = "test_name";
  }
}

interface ParentInterface {
  const CONSTANT_VALUE: GenericClass<User>;
}

interface SubInterface extends ParentInterface {}

class MyClass implements SubInterface {
  const CONSTANT_VALUE: GenericClass<User> = new GenericClass<User>(new User());
}

class SubClass extends MyClass {}

function main(): void {
  const sub = new SubClass();
}