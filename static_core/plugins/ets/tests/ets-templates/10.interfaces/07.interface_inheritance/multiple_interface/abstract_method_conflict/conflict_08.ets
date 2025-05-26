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
desc: Method with the same name in the parent class and interface, but with different return values.
---*/

abstract class Animal {
  abstract makeSound(): string;
}

interface Machine {
  makeSound(): string;
}

class Robot extends Animal implements Machine {
  makeSound(): string {
    console.log("Beep Beep!");
    return "Sound made!";
  }
}

function main(): void {
  const robot = new Robot();
  const result = robot.makeSound();
  assertEQ(result, "Sound made!");
}