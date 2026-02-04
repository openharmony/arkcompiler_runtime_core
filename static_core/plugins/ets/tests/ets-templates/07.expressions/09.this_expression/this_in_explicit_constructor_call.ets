/*
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

/*---
desc: This in direct call expression can only be used in explicit constructor call.
assert: The keyword this in a direct call expression this(arguments) can only be used in an explicit constructor call statement.
---*/

class A {
  x: int;
  y: int;

  constructor(x: int, y: int) {
    this.x = x;
    this.y = y;
  }

  constructor(x: int) {
    this(x, 0);
  }

  constructor() {
    this(0);
  }
}

function main(): void {
  let a1: A = new A(10, 20);
  arktest.assertEQ(a1.x, 10);
  arktest.assertEQ(a1.y, 20);

  let a2: A = new A(5);
  arktest.assertEQ(a2.x, 5);
  arktest.assertEQ(a2.y, 0);

  let a3: A = new A();
  arktest.assertEQ(a3.x, 0);
  arktest.assertEQ(a3.y, 0);
}
