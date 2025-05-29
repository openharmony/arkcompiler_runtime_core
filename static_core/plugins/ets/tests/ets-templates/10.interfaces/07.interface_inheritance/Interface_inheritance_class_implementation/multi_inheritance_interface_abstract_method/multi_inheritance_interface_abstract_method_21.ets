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
desc: Basic multi-interface multi-method implementation(boolean type).
---*/

interface IAnimal {
  num(): boolean;
  age(): boolean;
}

interface IFlyable {
  num1(): boolean;
  age1(): boolean;
}

class Bird implements IAnimal, IFlyable {
  num(): boolean {
    return true;
  }

  age(): boolean {
    return false;
  }

  num1(): boolean {
    return false;
  }

  age1(): boolean {
    return true;
  }
}

function main(): void {
  const bird = new Bird();
  const result = bird.num();
  const result1 = bird.age();
  const result2 = bird.num1();
  const result3 = bird.age1();
  assertEQ(result, true);
  assertEQ(result1, false);
  assertEQ(result2, false);
  assertEQ(result3, true);
}