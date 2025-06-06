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
desc: Multiple interface implementations with different parameters and return values.
---*/

interface ICalculator {
  add(a: number, b: number): number;
  subtract(a: number, b: number): number;
}

interface IStringManipulator {
  reverse(str: string): string;
  capitalize(str: string): string;
}

class Utility implements ICalculator, IStringManipulator {
  add(a: number, b: number): number {
    return a + b;
  }

  subtract(a: number, b: number): number {
    return a - b;
  }

  reverse(str: string): string {
    return str;
  }

  capitalize(str: string): string {
    return str;
  }
}

function main(): void {
  const util = new Utility();
  const result = util.add(3, 3);
  const result1 = util.subtract(4, 4);
  const result2 = util.reverse('hello');
  const result3 = util.capitalize('world');
  assertEQ(result, 6);
  assertEQ(result1, 0);
  assertEQ(result2, "hello");
  assertEQ(result3, 'world');
}