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
    A new class instance expression creates a new object that is an instance of the specified class
    and it is described in full details below.
---*/

class C {
  i: int = 0
  j: string = ""
  k: boolean = false
  constructor(i: int, j: string, k: boolean) {
    this.i = i
    this.j = j
    this.k = k
  }
}

function main() {
  let i: C = new C(5, "str", true)
  arktest.assertEQ(i.i, 5)
  arktest.assertEQ(i.j, "str")
  arktest.assertTrue(i.k)
}
