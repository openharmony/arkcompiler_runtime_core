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
    Any array type is a class type that has an appropriate class in the
    Standard Library. It means that array types are subtypes of Object.
---*/

enum Color { Red, Green, Blue }
type Z = Object|undefined

function main(): int {
  let suite = new arktest.ArkTestsuite("Array Assignable to Object")

  suite.addTest("byte[] assignable to Object", () => {
    let arr: byte[] = [3, 2, 1]
    let obj: Object = arr
    arktest.assertTrue(obj === arr)
  })

  suite.addTest("short[] assignable to Object", () => {
    let arr: short[] = [3, 2, 1]
    let obj: Object = arr
    arktest.assertTrue(obj === arr)
  })

  suite.addTest("int[] assignable to Object", () => {
    let arr: int[] = [3, 2, 1]
    let obj: Object = arr
    arktest.assertTrue(obj === arr)
  })

  suite.addTest("long[] assignable to Object", () => {
    let arr: long[] = [3, 2, 1]
    let obj: Object = arr
    arktest.assertTrue(obj === arr)
  })

  suite.addTest("float[] assignable to Object", () => {
    let arr: float[] = [3, 2, 1]
    let obj: Object = arr
    arktest.assertTrue(obj === arr)
  })

  suite.addTest("double[] assignable to Object", () => {
    let arr: double[] = [3, 1, 2]
    let obj: Object = arr
    arktest.assertTrue(obj === arr)
  })

  suite.addTest("char[] assignable to Object", () => {
    let arr: char[] = [c'A', c'B', c'C']
    let obj: Object = arr
    arktest.assertTrue(obj === arr)
  })

  suite.addTest("boolean[] assignable to Object", () => {
    let arr: boolean[] = [true, false, true]
    let obj: Object = arr
    arktest.assertTrue(obj === arr)
  })

  suite.addTest("enum[] assignable to Object", () => {
    let arr: Color[] = [Color.Red, Color.Green, Color.Blue]
    let obj: Object = arr
    arktest.assertTrue(obj === arr)
  })

  suite.addTest("Byte[] assignable to Object", () => {
    let arr: Byte[] = [new Byte((127).toByte()), new Byte((2).toByte()), new Byte((1).toByte())]
    let obj: Object = arr
    arktest.assertTrue(obj === arr)
  })

  suite.addTest("Error[] assignable to Object", () => {
    let arr: Error[] = [new Error("A"), new Error("B"), new Error("C")]
    let obj: Object = arr
    arktest.assertTrue(obj === arr)
  })

  suite.addTest("string[][] assignable to Object", () => {
    let arr: string[][] = new string[2][2]
    arr[1][1] = "ABC"
    let obj: Object = arr
    arktest.assertTrue(obj === arr)
  })

  suite.addTest("Z[][] assignable to Object", () => {
    let arr: Z[][] = new Z[2][2]
    let obj: Object = arr
    arktest.assertTrue(obj === arr)
  })

  return suite.run()
}
