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
desc: Verify whether the constant values genericclass<enum> remain consistent in the subclass interface.
---*/

class GenericClass<T> {
  public value: T;

  constructor(value: T) {
    this.value = value;
  }
}

enum MyConstants {
  enum_value = 42
}

interface ParentInterface {
  readonly CONSTANT_VALUE: GenericClass<MyConstants>;
}

interface SubInterface extends ParentInterface {}

class MyClass implements SubInterface {
  readonly CONSTANT_VALUE: GenericClass<MyConstants> = new GenericClass<MyConstants>(MyConstants.enum_value);
}

class SubClass extends MyClass {}

function main(): void {
  const sub = new SubClass();
  assertEQ(sub.CONSTANT_VALUE.value, MyConstants.enum_value);
}