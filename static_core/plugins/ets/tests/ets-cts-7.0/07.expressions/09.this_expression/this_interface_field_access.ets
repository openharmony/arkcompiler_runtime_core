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
desc: Interacting with interface members inside the interface method.
assert: The keyword this can access interface fields from within interface methods.
---*/

interface A {
  x: int;

  getX(): int {
    return this.x;
  }

  setX(value: int): void {
    this.x = value;
  }
}

class AImpl implements A {
  public x: int = 0;
}

function main(): void {
  let a: AImpl = new AImpl();
  arktest.assertEQ(a.getX(), 0);

  a.setX(11);
  arktest.assertEQ(a.getX(), 11);
  arktest.assertEQ(a.x, 11);

  a.setX(100);
  arktest.assertEQ(a.getX(), 100);
}
