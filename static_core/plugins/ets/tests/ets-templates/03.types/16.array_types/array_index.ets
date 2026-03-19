/*---
Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
    Array element is accessed by its index. The index is an integer number
    in the range from 0 to array length minus 1.
---*/

function main(): int {
  let suite = new arktest.ArkTestsuite("Array Index and Data Types")

  suite.addTest("Access by byte index", () => {
    let x: int[] = [1, 2, 3]
    let i: byte = 0
    arktest.assertEQ(x[i], 1)
  })

  suite.addTest("Access by short index", () => {
    let x: int[] = [1, 2, 3]
    let i: short = 1
    arktest.assertEQ(x[i], 2)
  })

  suite.addTest("Access by int index", () => {
    let x: int[] = [1, 2, 3]
    let i: int = 0
    arktest.assertEQ(x[i], 1)
  })

  suite.addTest("Array with union type elements", () => {
    let x: (int | string)[] = [4, "abc", 5]
    arktest.assertEQ(x.length, 3)
  })

  return suite.run()
}
