/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: Non-static setter for the private static field
---*/

class A {
  private static _age: int = 0;

  set age(value: int) {
    A._age = value;
  }

  get age(): int {
    return A._age;
  }
}

function main(): void {
  let i: A = new A()
  i.age = 5;
  arktest.assertEQ( i.age, 5 );
}
