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
desc: Basic multi-interface multi-method implementation.
---*/

interface IAnimal {
  eat(): string;
  sleep(): string;
}

interface IFlyable {
  fly(): string;
  land(): string;
}

class Bird implements IAnimal, IFlyable {
  eat() {
    return 'Bird is eating.';
  }

  sleep() {
    return 'Bird is sleeping.';
  }

  fly() {
    return 'Bird is flying.';
  }

  land() {
    return 'Bird has landed.';
  }
}

function main(): void {
  const bird = new Bird();
  const result = bird.eat();
  const result1 = bird.sleep();
  const result2 = bird.fly();
  const result3 = bird.land();
  assertEQ(result, 'Bird is eating.');
  assertEQ(result1, 'Bird is sleeping.');
  assertEQ(result2, 'Bird is flying.');
  assertEQ(result3, 'Bird has landed.');
}