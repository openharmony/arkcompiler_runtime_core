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
desc: The parent class overrides the default methods of the part inherited by
      the subinterface implemented by the subinterface of the child class.
---*/

interface Parent{
  speak(): number[] {
    return [1, 2, 3];
  }

  move(): number[] {
    return [4, 5, 6];
  }
}

interface Child extends Parent{}

class Vehicle implements Child {
  move(): number[] {
    return [1, 2, 3];
  }
}

class Car extends Vehicle {
  speak(): number[] {
    return [4, 5, 6];
  }
}

function main(): void {
  const car = new Car();
  assertEQ(car.speak()[1], 5);
  assertEQ(car.move()[1], 2);
}