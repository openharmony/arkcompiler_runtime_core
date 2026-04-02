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
desc: Using 'this' in generic interface.
assert: The keyword this works correctly in generic interface methods.
---*/

interface Container<T> {
  data: T;

  getData(): T {
    return this.data;
  }

  setData(val: T): void {
    this.data = val;
  }

  getSelf(): Container<T> {
    return this;
  }
}

class IntContainer implements Container<int> {
  public data: int = 0;
}

class StringContainer implements Container<string> {
  public data: string = "";
}

function main(): void {
  let intC: IntContainer = new IntContainer();
  intC.setData(11);
  arktest.assertEQ(intC.getData(), 11);

  let self: Container<int> = intC.getSelf();
  arktest.assertEQ(self, intC);

  let strC: StringContainer = new StringContainer();
  strC.setData("hello");
  arktest.assertEQ(strC.getData(), "hello");
  arktest.assertEQ(strC.getSelf(), strC);
}
