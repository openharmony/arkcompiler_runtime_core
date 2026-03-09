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
    Initializers of overridden fields are preserved for execution,
    and the initialization is normally performed in the context
    of superclass constructors.
---*/

let res: string = ""

class C {
  field: number = C.init_C()
  private static init_C() {
    res += ":init_in_C"
    return 123
  }
}

class D1 extends C {
  override field: number = D1.init_D1()
  private static init_D1() {
    res += ":init_in_D1"
    return 321
  }
}

class D2 extends D1 {
  field: number = D2.init_D2()
  private static init_D2() {
    res += ":init_in_D2"
    return 42
  }
}

function main(): void {
  let a: D2 = new D2()
  arktest.assertTrue(a.field instanceof number)
  arktest.assertEQ(a.field, 42)
  arktest.assertEQ(res, ":init_in_C:init_in_D1:init_in_D2")
  res = ""
}
