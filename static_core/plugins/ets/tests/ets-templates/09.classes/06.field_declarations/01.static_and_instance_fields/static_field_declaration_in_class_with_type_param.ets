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
    A static field is instantiated when the class is initialized.
    A static field can have only one instantiation, irrespective of how
    many instances of that class (even if zero) are eventually created.
    A compile-time error occurs if the name of a type parameter of the
    surrounding declaration is used as the type of a static field.
---*/

class A<T> {
  static x: int = 42
  y: T
  constructor(v: T){
    this.y = v
  }
}

function main() {
  arktest.assertTrue(A.x instanceof int)
  arktest.assertEQ(A.x, 42)
  let a: A<string> = new A<string>("xyz")
  arktest.assertTrue(A.x instanceof int)
  arktest.assertEQ(A.x, 42)
  arktest.assertTrue(a.y instanceof string)
  arktest.assertEQ(a.y, "xyz")
  let b: A<int> = new A<int>(41)
  arktest.assertTrue(A.x instanceof int)
  arktest.assertEQ(A.x, 42)
  arktest.assertTrue(b.y instanceof int)
  arktest.assertEQ(b.y, 41)
}
