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
desc: Methods with the same name in multiple interfaces, but with different parameters.
---*/

let result: string = "";
let result1: number = 0

interface Animal {
  makeSound(): void;
}

interface Machine {
  makeSound(volume: number): void;
}

class Robot implements Animal, Machine {
  makeSound(): void {
    result = "Beep";
  }

  makeSound(volume: number): void {
    result1 = 1 + volume;
  }
}

function main(): void {
  const robot = new Robot();
  (robot as Animal).makeSound(); 
  (robot as Machine).makeSound(10);
  assertEQ(result, "Beep");
  assertEQ(result1, 11);
}