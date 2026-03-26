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
desc: >-
    Static fields are declared with the modifier static. A static field is not part of a class instance. There is one
    copy of a static field irrespective of how many instances of the class (even if zero) are eventually created.
---*/

class A {
  static x: int = 40
  get x(): int {
    return A.x + 5
  }
  set x(v: int) {
    A.x = v + 10
  }
}

function main() {
  arktest.assertTrue(A.x instanceof int)
  arktest.assertEQ(A.x, 40)
  let a: A = new A()
  arktest.assertTrue(a.x instanceof int)
  arktest.assertEQ(a.x, 45)
  arktest.assertTrue(A.x instanceof int)
  arktest.assertEQ(A.x, 40)
  A.x = 42
  arktest.assertTrue(a.x instanceof int)
  arktest.assertEQ(a.x, 47)
  arktest.assertTrue(A.x instanceof int)
  arktest.assertEQ(A.x, 42)
  a.x = 45
  arktest.assertTrue(a.x instanceof int)
  arktest.assertEQ(a.x, 60)
  arktest.assertTrue(A.x instanceof int)
  arktest.assertEQ(A.x, 55)
  A.x = 40 // reset due to jit mode warmup
}
