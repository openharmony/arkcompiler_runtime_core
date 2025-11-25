/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: Literal types are aligned with some ArkTS literals (see Literals). Their names
  are same as the names of their values, i.e., literals. Only three literal types
  are supported - string, null and undefined.
name: infer_from_union_6
---*/

type number_tuple = [int, double]

type tuple = ["a", int]

type faa = FixedArray<"a">
type tuple1 = [faa, int]
const faa_value: faa = ["a"]

type arr_a = "a"[]
type tuple2 = [arr_a, int]
const arr_a_value: arr_a = ["a"]

function main(): void {
    test01()
    test02()
    test03()
    test04()
    test05()
    test06()
    test07()
    test08()
    test09()
    test10()
    test11()
    test12()
    test13()
    test14()
    test15()
    test16()
    test17()
    test18()
    test19()
    test20()
    test21()
    test22()
    test23()
    test24()
}

function test24(): void {
    let a: tuple2 | number_tuple = [arr_a_value, 1]
    arktest.assertTrue(a[0][0] instanceof string)
    arktest.assertEQ(a[0][0], "a")
}

function test23(): void {
    const a: tuple2 | number_tuple = [arr_a_value, 1]
    arktest.assertTrue(a[0][0] instanceof "a")
    arktest.assertEQ(a[0][0], "a")
}

function test22(): void {
    let a: tuple1 | number_tuple = [faa_value, 1]
    arktest.assertTrue(a[0][0] instanceof string)
    arktest.assertEQ(a[0][0], "a")
}

function test21(): void {
    const a: tuple1 | number_tuple = [faa_value, 1]
    arktest.assertTrue(a[0][0] instanceof "a")
    arktest.assertEQ(a[0][0], "a")
}

function test20(): void {
    let a: tuple | number_tuple = ["a", 1]
    arktest.assertTrue(a[0] instanceof string)
    arktest.assertEQ(a[0], "a")
}

function test19(): void {
    const a: tuple | number_tuple = ["a", 1]
    arktest.assertTrue(a[0] instanceof "a")
    arktest.assertEQ(a[0], "a")
}

function test18(): void {
    let a: tuple2 | int[] = [arr_a_value, 1]
    arktest.assertTrue(a[0][0] instanceof string)
    arktest.assertEQ(a[0][0], "a")
}

function test17(): void {
    const a: tuple2 | int[] = [arr_a_value, 1]
    arktest.assertTrue(a[0][0] instanceof "a")
    arktest.assertEQ(a[0][0], "a")
}

function test16(): void {
    let a: tuple1 | int[] = [faa_value, 1]
    arktest.assertTrue(a[0][0] instanceof string)
    arktest.assertEQ(a[0][0], "a")
}

function test15(): void {
    const a: tuple1 | int[] = [faa_value, 1]
    arktest.assertTrue(a[0][0] instanceof "a")
    arktest.assertEQ(a[0][0], "a")
}

function test14(): void {
    let a: tuple | int[] = ["a", 1]
    arktest.assertTrue(a[0] instanceof string)
    arktest.assertEQ(a[0], "a")
}

function test13(): void {
    const a: tuple | int[] = ["a", 1]
    arktest.assertTrue(a[0] instanceof "a")
    arktest.assertEQ(a[0], "a")
}

function test12(): void {
    let a: tuple2 | FixedArray<int> = [arr_a_value, 1]
    arktest.assertTrue(a[0][0] instanceof string)
    arktest.assertEQ(a[0][0], "a")
}

function test11(): void {
    const a: tuple2 | FixedArray<int> = [arr_a_value, 1]
    arktest.assertTrue(a[0][0] instanceof "a")
    arktest.assertEQ(a[0][0], "a")
}

function test10(): void {
    let a: tuple1 | FixedArray<int> = [faa_value, 1]
    arktest.assertTrue(a[0][0] instanceof string)
    arktest.assertEQ(a[0][0], "a")
}

function test09(): void {
    const a: tuple1 | FixedArray<int> = [faa_value, 1]
    arktest.assertTrue(a[0][0] instanceof "a")
    arktest.assertEQ(a[0][0], "a")
}

function test08(): void {
    let a: tuple | FixedArray<int> = ["a", 1]
    arktest.assertTrue(a[0] instanceof string)
    arktest.assertEQ(a[0], "a")
}

function test07(): void {
    const a: tuple | FixedArray<int> = ["a", 1]
    arktest.assertTrue(a[0] instanceof "a")
    arktest.assertEQ(a[0], "a")
}

function test06(): void {
    let a: tuple2 | Array<int> = [arr_a_value, 1]
    arktest.assertTrue(a[0][0] instanceof string)
    arktest.assertEQ(a[0][0], "a")
}

function test05(): void {
    const a: tuple2 | Array<int> = [arr_a_value, 1]
    arktest.assertTrue(a[0][0] instanceof "a")
    arktest.assertEQ(a[0][0], "a")
}

function test04(): void {
    let a: tuple1 | Array<int> = [faa_value, 1]
    arktest.assertTrue(a[0][0] instanceof string)
    arktest.assertEQ(a[0][0], "a")
}

function test03(): void {
    const a: tuple1 | Array<int> = [faa_value, 1]
    arktest.assertTrue(a[0][0] instanceof "a")
    arktest.assertEQ(a[0][0], "a")
}

function test02(): void {
    let a: tuple | Array<int> = ["a", 1]
    arktest.assertTrue(a[0] instanceof string)
    arktest.assertEQ(a[0], "a")
}

function test01(): void {
    const a: tuple | Array<int> = ["a", 1]
    arktest.assertTrue(a[0] instanceof "a")
    arktest.assertEQ(a[0], "a")
}
