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
desc: To verify that the duck class implements the ianimal and iswimmable interfaces.
---*/

interface IAnimal {
  speak(): string;
  move(): string;
}

interface ISwimmable {
  swim(): string;
}

abstract class AbstractBird implements IAnimal {
  abstract move(): string;
  abstract speak(): string;
}

class Duck extends AbstractBird implements ISwimmable {
  speak(): string {
    return "Duck quacks.";
  }

  swim(): string {
    return "Duck swims.";
  }

  move(): string {
    return "Bird is flying.";
  }
}

function main(): void {
  let duck = new Duck();
  const speakOutput = duck.speak();
  const moveOutput = duck.move();
  const swimOutput = duck.swim();
  assertEQ(speakOutput, "Duck quacks.");
  assertEQ(moveOutput, "Bird is flying.");
  assertEQ(swimOutput, "Duck swims.");
}