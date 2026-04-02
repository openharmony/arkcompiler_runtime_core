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
desc: Verify return of 'this' type for interfaces.
assert: The keyword this should return the interface type when used in an interface method.
---*/

interface Fluent {
  x: int;

  setValue(val: int): Fluent {
    this.x = val;
    return this;
  }

  getSelf(): Fluent {
    return this;
  }
}

class FluentImpl implements Fluent {
  public x: int = 0;
}

function main(): void {
  let f: FluentImpl = new FluentImpl();
  let result: Fluent = f.setValue(11);
  arktest.assertEQ(result, f);
  arktest.assertEQ(f.x, 11);

  let self: Fluent = f.getSelf();
  arktest.assertEQ(self, f);

  // Test method chaining
  let chained: Fluent = f.setValue(10).setValue(20).getSelf();
  arktest.assertEQ(chained, f);
  arktest.assertEQ(f.x, 20);
}
