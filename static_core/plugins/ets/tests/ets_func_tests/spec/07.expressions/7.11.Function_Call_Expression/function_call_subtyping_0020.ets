/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc: Function Call Expression Subtyping
name: function_call_subtyping_0000.ets
spec: 7.11
---*/

class A {
  countA : int;
  
  constructor() {
    this.countA = 6
  }
}
class B extends A {
  countB : int;

  constructor() {
    super()
    this.countB = 28
  }
}
class D extends B {
  countD : int;

  constructor() {
    super()
    this.countD = 496
  }
}

function f(): D {
  return new D();
}

function main() {
  let x : A = f()
  assertEQ(x.countA, 6)

  let y : B = f()
  assertEQ(y.countA, 6)
  assertEQ(y.countB, 28)

  let z : D = f()
  assertEQ(y.countA, 6)
  assertEQ(y.countB, 28)
  assertEQ(y.countD, 496)
}
