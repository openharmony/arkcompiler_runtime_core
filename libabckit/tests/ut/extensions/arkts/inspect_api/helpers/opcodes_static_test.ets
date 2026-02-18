/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

class Person {
  public name: string;

  constructor(n: string) {
    this.name = n;
  }
}

function foo(x: number, y: number): number {
  let z = x + y;
  if (z > 10) {
    z = z - 1;
  } else {
    z = z + 1;
  }
  return z;
}

function main() {
  // new object + strings
  let p = new Person("Bob");

  // checkcast-like path
  let q = p as Person;

  // instanceof
  let b = q instanceof Person;

  // literal array / array ops
  let arr = [1, 2, 3, 4];
  let sum: number = 0;
  for (let i = 0; i < arr.length; i++) {
    sum += arr[i];
  }

  // try/catch + call + branches
  try {
    if (b) {
      sum = foo(sum, 2);
    }
  } catch (e) {
    sum = -1;
  }

  console.log(p.name);
  console.log(sum);
}

