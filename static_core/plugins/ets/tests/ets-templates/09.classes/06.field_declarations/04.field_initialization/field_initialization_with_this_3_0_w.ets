/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: Non-static field initialization with keyword 'this'.
assert: >-
    If the instance field initializer expression uses
    this or super directly in any form,then a compile-time warning occurs.
expected_out: {"compiler": "Warning W0010: The instance field initializer expression cannot use the this."}
---*/

function gr(this: A): int {
    return this.y
}

class A {
  public y: int = 5
  public x: int = this.gr()

}

function main(): void {
    let instance: A = new A()

    arktest.assertEQ(instance.x, 5)
}
