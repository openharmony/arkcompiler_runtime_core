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
desc: This in lambda body denotes the same value as in surrounding context.
assert: The parameter this in a lambda body and in the surrounding context denote the same value.
---*/

class A {
  x: int = 0;

  method(): void {
    let lambda = (): void => {
      this.x = 10; // this refers to the enclosing A instance
    };
    lambda();
  }

  getX(): int {
    return this.x;
  }
}

function main(): void {
  let a: A = new A();
  arktest.assertEQ(a.getX(), 0);

  a.method();
  arktest.assertEQ(a.getX(), 10);
}
