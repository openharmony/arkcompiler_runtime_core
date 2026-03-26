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
desc: An array is an object which contains elements of a similar data type.
---*/

function main(): int {
  let suite = new arktest.ArkTestsuite("Array Type Literals")

  suite.addTest("Empty int array", () => {
    let x: int[] = []
    arktest.assertTrue(x.length == 0)
  })

  suite.addTest("Single element int array", () => {
    let x: int[] = [1]
    arktest.assertTrue(x.length == 1 && x[0] == 1)
  })

  suite.addTest("Multi element int array", () => {
    let x: int[] = [1, 2, 3]
    arktest.assertTrue(x.length == 3 && x[2] == 3)
  })

  suite.addTest("Double array", () => {
    let x: double[] = [3.1415926, 35.56]
    arktest.assertTrue(x.length == 2 && x[1] == 35.56)
  })

  return suite.run()
}
