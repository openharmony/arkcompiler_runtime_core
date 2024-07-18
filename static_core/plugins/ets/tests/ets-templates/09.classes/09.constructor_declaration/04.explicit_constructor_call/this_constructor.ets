/*---
Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
desc: Alternate constructor calls that begin with the keyword this
---*/

class A {
  public x: int;

  public constructor() {
    this(5);
  }

  public constructor(a: int) {
    this.x = a;
  }
}

function main(): void {
  let i1: A = new A();
  let i2: A = new A(7);

  assert i1.x == 5;
  assert i2.x == 7;
}
