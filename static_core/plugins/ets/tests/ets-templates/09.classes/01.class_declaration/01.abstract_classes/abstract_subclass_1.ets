/*---
Copyright (c) 2024 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: >-
    Subclasses of an abstract class can be non-abstract or in turn abstract.
    A non-abstract subclass of an abstract superclass can be instantiated.
    As a result, a constructor for the abstract class, and field initializers
     for non-static fields of that class are executed.
---*/

abstract class A {
  f: int;
  constructor() {
    this.f = 3;
  }
}

class B extends A {
  f1: int;
  constructor() {
    this.f1 = this.f + 2;
  }
}

function main(): void {
  let c: B = new B();
  assert c.f1 == 5;
}
