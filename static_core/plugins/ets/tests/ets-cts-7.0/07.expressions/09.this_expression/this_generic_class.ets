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
desc: Using 'this' in generic class.
assert: The keyword this works correctly in generic class methods.
---*/

class A<T> {
  private value: T;

  constructor(val: T) {
    this.value = val;
  }

  public getValue(): T {
    return this.value;
  }

  public setValue(val: T): void {
    this.value = val;
  }

  public getSelf(): A<T> {
    return this;
  }
}

function main(): void {
  let a: A<int> = new A<int>(11);
  arktest.assertEQ(a.getValue(), 11);

  a.setValue(100);
  arktest.assertEQ(a.getValue(), 100);

  let self: A<int> = a.getSelf();
  arktest.assertEQ(self, a);
  arktest.assertEQ(self.getValue(), 100);

  let strA: A<string> = new A<string>("hello");
  arktest.assertEQ(strA.getValue(), "hello");
  arktest.assertEQ(strA.getSelf(), strA);
}
