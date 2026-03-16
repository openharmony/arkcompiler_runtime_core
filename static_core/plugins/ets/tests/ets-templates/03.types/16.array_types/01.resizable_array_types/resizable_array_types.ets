/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
desc: >-
    Resizable array type with elements of type T can be declared
    as Array<T> for various element types.
---*/

function main(): int {
  let suite = new arktest.ArkTestsuite("Resizable Array Element Types")

  suite.addTest("Array<byte>", () => {
    let arr: Array<byte> = [1]
    arktest.assertEQ(arr.length, 1)
  })

  suite.addTest("Array<short>", () => {
    let arr: Array<short> = [1]
    arktest.assertEQ(arr.length, 1)
  })

  suite.addTest("Array<int>", () => {
    let arr: Array<int> = [1]
    arktest.assertEQ(arr.length, 1)
  })

  suite.addTest("Array<long>", () => {
    let arr: Array<long> = [1]
    arktest.assertEQ(arr.length, 1)
  })

  suite.addTest("Array<bigint>", () => {
    let arr: Array<bigint> = [1n]
    arktest.assertEQ(arr.length, 1)
  })

  suite.addTest("Array<float>", () => {
    let arr: Array<float> = [1.0f]
    arktest.assertEQ(arr.length, 1)
  })

  suite.addTest("Array<double>", () => {
    let arr: Array<double> = [1.0]
    arktest.assertEQ(arr.length, 1)
  })

  suite.addTest("Array<boolean>", () => {
    let arr: Array<boolean> = [true]
    arktest.assertEQ(arr.length, 1)
  })

  suite.addTest("Array<char>", () => {
    let arr: Array<char> = [c'a']
    arktest.assertEQ(arr.length, 1)
  })

  suite.addTest("Array<string>", () => {
    let arr: Array<string> = ['abc']
    arktest.assertEQ(arr.length, 1)
  })

  suite.addTest("Array<Any>", () => {
    let arr: Array<Any> = [1]
    arktest.assertEQ(arr.length, 1)
  })

  suite.addTest("Array<Object>", () => {
    let arr: Array<Object> = [new Object()]
    arktest.assertEQ(arr.length, 1)
  })

  suite.addTest("Array<null>", () => {
    let arr: Array<null> = [null]
    arktest.assertEQ(arr.length, 1)
  })

  suite.addTest("Array<undefined>", () => {
    let arr: Array<undefined> = [undefined]
    arktest.assertEQ(arr.length, 1)
  })

  suite.addTest("Array<int | string> with int", () => {
    let arr: Array<int | string> = [1]
    arktest.assertEQ(arr.length, 1)
  })

  suite.addTest("Array<int | string> with string", () => {
    let arr: Array<int | string> = ['abc']
    arktest.assertEQ(arr.length, 1)
  })

  suite.addTest("Array<int[]>", () => {
    let arr: Array<int[]> = [[1, 0]]
    arktest.assertEQ(arr.length, 1)
  })

  suite.addTest("Array<Array<int>>", () => {
    let arr: Array<Array<int>> = [[1]]
    arktest.assertEQ(arr.length, 1)
  })

  suite.addTest("Array<FixedArray<int>>", () => {
    let arr: Array<FixedArray<int>> = [[1, 0]]
    arktest.assertEQ(arr.length, 1)
  })

  suite.addTest("Array<() => void>", () => {
    let arr: Array<() => void> = [(): void => {}]
    arktest.assertEQ(arr.length, 1)
  })

  suite.addTest("Array<'abc' | 'def'>", () => {
    let arr: Array<'abc' | 'def'> = ['def']
    arktest.assertEQ(arr.length, 1)
  })

  return suite.run()
}
