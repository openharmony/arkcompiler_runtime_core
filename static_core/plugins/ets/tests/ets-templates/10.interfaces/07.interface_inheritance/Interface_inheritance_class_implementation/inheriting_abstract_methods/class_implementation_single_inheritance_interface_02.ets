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
desc: To verify that the cat class implements the ianimal interface.
---*/

interface IAnimal {
  speak(): string;
  move(): string;
}

abstract class AbstractMammal implements IAnimal {
  move(): string {
    return "Mammal is walking.";
  }

  abstract speak(): string;
}

class Cat extends AbstractMammal {
  speak(): string {
    return "Cat meows.";
  }
}

function main(): void {
  let cat = new Cat();
  const speakOutput = cat.speak();
  const moveOutput = cat.move();
  assertEQ(speakOutput, "Cat meows.");
  assertEQ(moveOutput, "Mammal is walking.");
}