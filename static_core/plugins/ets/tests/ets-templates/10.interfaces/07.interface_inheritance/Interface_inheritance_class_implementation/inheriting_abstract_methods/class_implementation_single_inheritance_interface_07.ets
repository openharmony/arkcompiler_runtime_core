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
desc: Inheritance of the subclass to the default implementation of the parent class.
---*/

interface IAnimal {
  move(): string;
}

abstract class AbstractMammal implements IAnimal {
  move(): string {
    return "Mammal is walking.";
  }
}

class Dog extends AbstractMammal {}

function main(): void {
  let dog = new Dog();
  let result = dog.move();
  assertEQ(result, "Mammal is walking.");
}