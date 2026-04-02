/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: Resizable array type basic operations, multidimensional arrays, union and class element types, function parameters, and Array<T> syntax.
---*/

class PersonStr {
  name: string
  constructor(name: string) { this.name = name }
}

class PersonNum {
  name: number
  constructor(name: number) { this.name = name }
}

class PersonBool {
  name: boolean
  constructor(name: boolean) { this.name = name }
}

type Matrix = number[][]
type MatrixStr = string[][]
type ThreeDArray = number[][][]
type ThreeDArrayStr = string[][][]

function printArrayStr(arr: string[]): string[] { return arr }
function printArrayNum(arr: number[]): number[] { return arr }
function printArrayBool(arr: boolean[]): boolean[] { return arr }

function growArray(): void {
  let a: number[] = [0, 1, 2]
  a.length = 5
}

function main(): int {
  let suite = new arktest.ArkTestsuite("Resizable Array Types");

  suite.addTest("Access element from number[]", () => {
    let a: number[] = [1, 2, 3]
    arktest.assertTrue(a[2] === 3)
  });

  suite.addTest("Assign value to element in number[]", () => {
    let a: number[] = [0, 0, 0, 0, 0]
    a[2] = 3
    arktest.assertEQ(a[2], 3)
  });

  suite.addTest("Get length of number[]", () => {
    let a: number[] = [0, 1, 2, 3]
    arktest.assertEQ(a.length, 4)
  });

  suite.addTest("Get length of string[]", () => {
    let a: string[] = ["aa", "bb", "cc", "dd", "ee"]
    arktest.assertEQ(a.length, 5)
  });

  suite.addTest("Shrink array by reducing length", () => {
    let a: number[] = [0, 1, 2, 3, 4]
    a.length = 3
    arktest.assertNE(a.length, 5)
    arktest.assertEQ(a.length, 3)
  });

  suite.addTest("Increasing array length throws RangeError", () => {
    let checkException = (e: Error): boolean => { return (e instanceof RangeError) }
    arktest.expectThrow(() => growArray(), checkException)
  });

  suite.addTest("Create empty array", () => {
    let a: number[] = []
    arktest.assertEQ(a.length, 0)
  });

  suite.addTest("Create 2D number[][] and access element", () => {
    let m: Matrix = [[1, 2], [3, 4]]
    arktest.assertEQ(m[0][1], 2)
  });

  suite.addTest("Access 2D number[][] element [1][1]", () => {
    let m: Matrix = [[1, 2], [3, 4]]
    arktest.assertEQ(m[1][1], 4)
  });

  suite.addTest("Create 2D string[][] and access element", () => {
    let m: MatrixStr = [["aa", "bb"], ["cc", "dd"]]
    arktest.assertEQ(m[0][1], "bb")
  });

  suite.addTest("Access 2D string[][] element [1][1]", () => {
    let m: MatrixStr = [["aa", "bb"], ["cc", "dd"]]
    arktest.assertEQ(m[1][1], "dd")
  });

  suite.addTest("Create 3D number[][][] and access element", () => {
    let a: ThreeDArray = [[[1, 2], [3, 4]], [[5, 6], [7, 8]]]
    arktest.assertEQ(a[0][1][0], 3)
  });

  suite.addTest("Access 3D number[][][] element [1][1][0]", () => {
    let a: ThreeDArray = [[[1, 2], [3, 4]], [[5, 6], [7, 8]]]
    arktest.assertEQ(a[1][1][0], 7)
  });

  suite.addTest("Access 3D string[][][] element [1][1][0]", () => {
    let a: ThreeDArrayStr = [[["aa", "bb"], ["cc", "dd"]], [["ee", "ff"], ["gg", "hh"], ["ii", "jj"]]]
    arktest.assertEQ(a[1][1][0], "gg")
  });

  suite.addTest("Access 3D string[][][] element [0][1][0]", () => {
    let a: ThreeDArrayStr = [[["aa", "bb"], ["cc", "dd"]], [["ee", "ff"], ["gg", "hh"], ["ii", "jj"]]]
    arktest.assertEQ(a[0][1][0], "cc")
  });

  suite.addTest("number[] assignable to Object", () => {
    let arr: number[] = [1, 2, 3]
    let o: Object = arr
    arktest.assertEQ(o, arr)
  });

  suite.addTest("string[] assignable to Object", () => {
    let arr: string[] = ["aa", "bb", "cc"]
    let o: Object = arr
    arktest.assertEQ(o, arr)
  });

  suite.addTest("Union (number | string)[] element types", () => {
    let arr: (number | string)[] = [1, "test"]
    arktest.assertEQ(typeof arr[0], "number")
    arktest.assertEQ(typeof arr[1], "string")
  });

  suite.addTest("Union (number | boolean)[] element types", () => {
    let arr: (number | boolean)[] = [1, true]
    arktest.assertEQ(typeof arr[1], "boolean")
  });

  suite.addTest("Union (string | boolean)[] element types", () => {
    let arr: (string | boolean)[] = ["test", false]
    arktest.assertEQ(typeof arr[0], "string")
  });

  suite.addTest("Union (number | string | boolean)[] element types", () => {
    let arr: (number | string | boolean)[] = [1, "test", false]
    arktest.assertEQ(typeof arr[0], "number")
    arktest.assertEQ(typeof arr[1], "string")
    arktest.assertEQ(typeof arr[2], "boolean")
  });

  suite.addTest("Class object array with string attribute", () => {
    let arr: PersonStr[] = [new PersonStr("Alice"), new PersonStr("Bob")]
    arktest.assertEQ(typeof arr[0].name, "string")
  });

  suite.addTest("Class object array with number attribute", () => {
    let arr: PersonNum[] = [new PersonNum(2), new PersonNum(13)]
    arktest.assertEQ(typeof arr[1].name, "number")
  });

  suite.addTest("Class object array with boolean attribute", () => {
    let arr: PersonBool[] = [new PersonBool(true), new PersonBool(false)]
    arktest.assertEQ(typeof arr[1].name, "boolean")
  });

  suite.addTest("string[] as function parameter", () => {
    let a: string[] = ["apple", "banana", "cherry"]
    arktest.assertTrue(a === printArrayStr(a))
  });

  suite.addTest("number[] as function parameter", () => {
    let a: number[] = [1, 2, 3]
    arktest.assertTrue(a === printArrayNum(a))
  });

  suite.addTest("boolean[] as function parameter", () => {
    let a: boolean[] = [true, false]
    arktest.assertTrue(a === printArrayBool(a))
  });

  suite.addTest("Array<number> element access", () => {
    let a: Array<number> = [4, 5, 6]
    arktest.assertEQ(a[0], 4)
  });

  suite.addTest("Array<number> element assignment", () => {
    let a: Array<number> = [0, 0, 0, 0, 0]
    a[0] = 4
    arktest.assertEQ(a[0], 4)
  });

  suite.addTest("Array<number> length", () => {
    let a: Array<number> = [0, 1, 2, 3]
    arktest.assertEQ(a.length, 4)
  });

  suite.addTest("Array<string> assignable to Object", () => {
    let arr: Array<string> = ["aa", "bb", "cc"]
    let o: Object = arr
    arktest.assertEQ(o, arr)
  });

  suite.addTest("Array<number> assignable to Object", () => {
    let arr: Array<number> = [1, 2, 3]
    let o: Object = arr
    arktest.assertEQ(o, arr)
  });

  suite.addTest("Array<boolean> element access", () => {
    let a: Array<boolean> = [true, false, true]
    arktest.assertEQ(a[1], false)
  });

  suite.addTest("Array<int> element access and assignment", () => {
    let a: Array<int> = [10, 20, 30]
    a[1] = 25
    arktest.assertEQ(a[1], 25)
  });

  suite.addTest("Array<char> element access", () => {
    let a: Array<char> = [c'A', c'B', c'C']
    arktest.assertEQ(a[2], c'C')
  });

  suite.addTest("Array<double> element access", () => {
    let a: Array<double> = [1.1, 2.2, 3.3]
    arktest.assertEQ(a[0], 1.1)
  });

  suite.addTest("Array<long> element access", () => {
    let a: Array<long> = [100, 200, 300]
    arktest.assertEQ(a[2], 300)
  });

  suite.addTest("Array<boolean> assignable to Object", () => {
    let arr: Array<boolean> = [true, false]
    let o: Object = arr
    arktest.assertEQ(o, arr)
  });

  return suite.run();
}
