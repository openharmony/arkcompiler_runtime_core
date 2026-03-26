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
desc: Verify return of 'this' type for classes.
assert: The keyword this should return the class type when used in a class method.
---*/

class A {
  public x: int;

  public setValue(val: int): A {
    this.x = val;
    return this;
  }

  public getSelf(): A {
    return this;
  }
}

function main(): void {
  let a: A = new A();
  let result: A = a.setValue(15);
  arktest.assertEQ(result, a);
  arktest.assertEQ(result.x, 15);

  let self: A = a.getSelf();
  arktest.assertEQ(self, a);

  // Test method chaining
  let chained: A = a.setValue(10).setValue(20).getSelf();
  arktest.assertEQ(chained, a);
  arktest.assertEQ(chained.x, 20);
}
