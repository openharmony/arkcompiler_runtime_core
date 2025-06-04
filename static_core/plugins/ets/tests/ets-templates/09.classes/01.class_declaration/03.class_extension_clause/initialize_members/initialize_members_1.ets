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
desc: To verify that the parent class member variables whose type is string are correctly initialized.
---*/

class Animal {
  public value: string;

  constructor(value: string) {
    this.value =value;
  }
}

class Dog extends Animal {
  constructor(value: string) {
    super(value);
  }
}

function main(): void {
  const myAnimal = new Animal('def');
  assertEQ(myAnimal.value, 'def');
  const myDog = new Dog('abc');
  assertEQ(myDog.value, 'abc');
}