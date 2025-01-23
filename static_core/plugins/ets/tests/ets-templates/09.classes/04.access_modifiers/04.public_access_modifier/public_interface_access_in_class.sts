/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: Check interface with public access modifier in class.
assert: Member or constructor with public modifier can be accessed everywhere.
tags: [negative, compile-only]
---*/

class A {
  interface I { fee(): int }

  class B implements I {
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

  assertEQ( instance.foo(), 1 );
}
