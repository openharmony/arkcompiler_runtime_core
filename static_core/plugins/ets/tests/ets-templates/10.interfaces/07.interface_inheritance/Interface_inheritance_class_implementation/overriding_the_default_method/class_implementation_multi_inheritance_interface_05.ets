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
desc: Abstract class overrides some default methods inherited by sub-interfaces.
---*/

interface Parent{
  speak(): string {
    return "speak";
  }

  move(): string {
    return "move";
  }
}

interface Child extends Parent{}

abstract class AbstractAnimal implements Child {
  speak(): string {
    return "speaking";
  }
}

class Animal extends AbstractAnimal {}

function main(): void {
  const animal = new Animal();
  assertEQ(animal.speak(), "speaking");
  assertEQ(animal.move(), "move");
}