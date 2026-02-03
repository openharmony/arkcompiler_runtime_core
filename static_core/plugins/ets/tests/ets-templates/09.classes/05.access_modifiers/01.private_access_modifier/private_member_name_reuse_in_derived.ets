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
desc: >-
    A derived class can declare a field or method with the same name as a private
    member in the base class, since the private member is inaccessible.
---*/

class C {
  private count: number = 1
  private hello() {}
  getCount(): number {
    return this.count
  }
  sayHello() {
    this.hello()
  }
}

class D2 extends C {
  count: number = 100  // OK to reuse name since 'count' in base is inaccessible
  hello() {}           // OK to reuse name since 'hello' in base is inaccessible
  foo() {
    this.count++       // OK - accesses D2's own count field
    this.hello()       // OK - accesses D2's own hello method
  }
}

function main(): void {
  let d: D2 = new D2();
  arktest.assertEQ(d.count, 100);
  d.foo();
  arktest.assertEQ(d.count, 101);
}
