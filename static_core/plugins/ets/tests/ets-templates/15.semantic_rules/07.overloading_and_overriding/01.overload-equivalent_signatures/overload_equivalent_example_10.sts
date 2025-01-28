/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc: The following signatures are not overload-equivalent.
---*/

let res: int = 0;

class A {}

class B extends A {
  foo(x: A): void {
    res = 1;
  }

  foo(y: B): void {
    res = 2;
  }
}

function main(): void {
  let b = new B();
  b.foo(new A());
  assertTrue(res == 1);
  b.foo(b);
  assertTrue(res == 2);
}
