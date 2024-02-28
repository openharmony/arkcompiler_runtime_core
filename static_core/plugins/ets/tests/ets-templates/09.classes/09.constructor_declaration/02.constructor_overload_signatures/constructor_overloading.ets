/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
desc: Class constructors overloading
assert: Overloading of constructors is identical in behavior to overloading of methods. The overloading is resolved at compile time by each class instance creation expression.
---*/

class A {
  public x: int;

  public constructor() {
    this.x = 5;
  }

  public constructor(a: int) {
    this.x = a;
  }

  public constructor(a: int, b: char) {
    this.x = 10;
  }

  public constructor(a: int, b: char, d: Boolean) {
    this.x = 15;
  }
}

function main(): void {
  let i1: A = new A();
  let i2: A = new A(7);
  let i3: A = new A(7, c's');
  let i4: A = new A(7, c's', true);

  assert i1.x == 5;
  assert i2.x == 7;
  assert i3.x == 10;
  assert i4.x == 15;
}
