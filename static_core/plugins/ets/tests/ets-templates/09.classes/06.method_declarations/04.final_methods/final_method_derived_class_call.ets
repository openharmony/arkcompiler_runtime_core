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
desc: Checks that final method can be called from another method inside the same class.
tags: [compile-only]
---*/

class A {
  res: int = 0;
  
  final method1(): void {
    this.res = 1;
  }
}

clss B extends A {
  method2(): void {
    this.method1();
  }
}

function main(): void {
  let a: A = new A();
  a.method2();
  assert a.res == 1;
}
