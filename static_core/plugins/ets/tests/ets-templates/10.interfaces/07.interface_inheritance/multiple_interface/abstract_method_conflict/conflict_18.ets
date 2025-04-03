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
desc: Method with the same name in multiple interfaces, 
      the child class chooses to implement one, and calls the implementation of the parent class.
---*/

let result: string = "";

interface Animal {
  makeSound(): void;
}

interface Machine {
  makeSound(): void;
}

class Robot implements Animal {
  makeSound(): void {
    result = "Beep";
  }
}

class AdvancedRobot extends Robot implements Machine {
  makeSound(): void {
    super.makeSound(); 
    result = "Advanced Beep";
  }
}

function main(): void {
  const advancedRobot = new AdvancedRobot();
  advancedRobot.makeSound();
  assertEQ(result, "Advanced Beep");
}