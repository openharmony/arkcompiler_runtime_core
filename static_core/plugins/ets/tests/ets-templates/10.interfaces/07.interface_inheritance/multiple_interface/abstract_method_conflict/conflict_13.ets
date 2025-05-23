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
desc: The same-named method in the parent class and interface, 
      and the child class chooses to call the default implementation of the interface.
---*/

let result: string = "";

abstract class Animal {
  makeSound(): void {
    console.log("Animal sound");
  }
}

interface Machine {
  makeSound(): void;
}

class BaseMachine implements Machine {
  makeSound(): void {
    result = "Machine";
  }
}

class Robot extends Animal implements Machine {
  makeSound(): void {
    super.makeSound(); 
    new BaseMachine().makeSound();
    result = "Beep";
  }
}

function main(): void {
  const robot = new Robot();
  robot.makeSound();
  assertEQ(result, "Beep");
}