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
desc: Check interface with private access modifier in class.
assert: Member or constructor with private modifier can be accessed only eithin their declaring class.
---*/

class A {
  private interface I { fee(): int }

  private inner class B implements I {
    public override fee(): int {
      return 1;
    }
  }

  public foo(): int {
    return new B().fee();
  }
}

function main(): void {
  let instance: A = new A();

  assert instance.foo() == 1;
}
