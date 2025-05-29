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
desc: The method with the same name has the same parameter type but different return types, redefine the calculated method.
---*/

interface Parent1 {
  calculate(value: number): number;
}

interface Parent2 {
  calculate(value: number): string;
}

class ChildClass {
  private parent1: Parent1;
  private parent2: Parent2;

  constructor(parent1: Parent1, parent2: Parent2) {
    this.parent1 = parent1;
    this.parent2 = parent2;
  }

  calculate(value: number): number | string {
    if (value > 10) {
      return this.parent1.calculate(value);
    } else {
      return this.parent2.calculate(value);
    }
  }
}

class Parent1Impl implements Parent1 {
  calculate(value: number): number {
    return value * 2;
  }
}

class Parent2Impl implements Parent2 {
  calculate(value: number): string {
    return `Result: ${value}`;
  }
}

function main(): void {
  const parent1 = new Parent1Impl();
  const parent2 = new Parent2Impl();
  const child = new ChildClass(parent1, parent2);

  console.log(child.calculate(5));
  assertEQ(child.calculate(15), 30);
}