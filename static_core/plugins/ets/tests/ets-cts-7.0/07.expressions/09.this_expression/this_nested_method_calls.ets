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
desc: Nested method calls through this.
assert: The keyword this can be used in nested method calls.
---*/

class A {
  private x: int;

  constructor(x: int) {
    this.x = x;
  }

  public calculate(): int {
    return this.getValue() + this.getDouble();
  }

  public getValue(): int {
    return this.x;
  }

  public getDouble(): int {
    return this.getValue() * 2;
  }
}

function main(): void {
  let a: A = new A(10);
  arktest.assertEQ(a.getValue(), 10);
  arktest.assertEQ(a.getDouble(), 20);
  arktest.assertEQ(a.calculate(), 30);  // 10 + 20

  let a2: A = new A(5);
  arktest.assertEQ(a2.calculate(), 15);  // 5 + 10
}
