/*---
Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
    The transitive closure of the direct superinterface relationship results in the superinterface relationship.
---*/

interface I {
  foo() { return 2 }
}
interface II extends I {}
class C implements II {
  foo() { return 3 }
}
function boo(i: I) {
  return i.foo()
}

function main() {
  let i1: I = {}
  let i2: II = {}
  let i3: C = new C()
  arktest.assertEQ(boo(i1), 2)
  arktest.assertEQ(boo(i2), 2)
  arktest.assertEQ(boo(i3), 3)
}
