/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
import * as ArrayTest from "@generated/array_test";
import * as System from "std/core/System";

loadLibrary("ani_array");

function sumArray() {
    let numbers: int[] = [1, 2, 3, 4, 5];
    let sum = ArrayTest.sumArray(numbers, 100)
    console.log("sum is " + sum)
    assertEQ(sum, 115)
}

function getArrayValue() {
    let nums: long[] = [35, 45, 55];
    let res = ArrayTest.getArrayValue(nums, 2)
    console.log("index 2 is " + res)
    assertEQ(res, 55)
}

function toStingArray() {
    let nums: int[] = [35, 45, 55];
    let res = ArrayTest.toStingArray(nums)
    console.log("new array is " + res)
    assertEQ(res.length, nums.length)
    for (let i = 0; i < res.length; i++) {
        assertEQ(res[i], `${nums[i]}`)
    }
}

function makeIntArray() {
    let arr = ArrayTest.makeIntArray(5, 3)
    console.log("new array is " + arr)
    assertEQ(arr.length, 3)
    for (let i = 0; i < arr.length; i++) {
        assertEQ(arr[i], 5)
    }
}

function makeEnumArray() {
    let arr = ArrayTest.makeEnumArray(ArrayTest.Color.GREEN, 5)
    console.log("new array is " + arr)
    assertEQ(arr.length, 5)
    for (let i = 0; i < arr.length; i++) {
        assertEQ(arr[i], ArrayTest.Color.GREEN)
    }
}

function makeRecordArray() {
    let arr = ArrayTest.makeRecordArray("k", 128, 3)
    console.log("new array is " + arr)
    assertEQ(arr.length, 3)
    for (let i = 0; i < arr.length; i++) {
        assertEQ(arr[i]["k"], 128)
    }
}

function makeStructArray() {
    let arr = ArrayTest.makeStructArray("a", "b", 5, 3)
    console.log("new array is " + arr)
    assertEQ(arr.length, 3)
    for (let i = 0; i < arr.length; i++) {
        assertEQ(arr[i].a, "a")
        assertEQ(arr[i].b, "b")
        assertEQ(arr[i].c, 5)
    }
}

function makeIntArray2() {
    let arr = ArrayTest.makeIntArray(5, 3);
    let arr2 = ArrayTest.makeIntArray2(arr, 3);
    assertEQ(arr2.length, 3);
    for (let i = 0; i < arr2.length; i++) {
        assertEQ(arr2[i].length, 3)
    }
}

function changeEnumArray() {
    let arr = ArrayTest.makeEnumArray(ArrayTest.Color.GREEN, 5);
    let arr2 = ArrayTest.changeEnumArray(arr, ArrayTest.Color.BLUE);
    assertEQ(arr.length, arr2.length)
    for (let i = 0; i < arr2.length; i++) {
        assertEQ(arr2[i], ArrayTest.Color.BLUE)
    }
}

function changeRecordArray() {
    let arr = ArrayTest.makeRecordArray("k", 128, 3)
    let arr2 = ArrayTest.changeRecordArray(arr, "v", 21);
    assertEQ(arr.length, arr2.length)
    for (let i = 0; i < arr2.length; i++) {
        assertEQ(arr2[i]["v"], 21)
    }
}

function changeStructArray() {
    let arr = ArrayTest.makeStructArray("a", "b", 5, 3)
    let arr2 = ArrayTest.changeStructArray(arr, "aa", "bb", 3);
    assertEQ(arr.length, arr2.length)
    for (let i = 0; i < arr2.length; i++) {
        assertEQ(arr2[i].a, "aa")
        assertEQ(arr2[i].b, "bb")
        assertEQ(arr2[i].c, 3)
    }
}

function fetchBinaryData() {
    ArrayTest.fetchBinaryData(5, (error: Error, data?: float[]) => {
        if (!error.message) {
            console.log("main finsih test async success ");
            assertEQ(data?.length, 5);
        } else {
            console.log("main finsih test async ERROR ", error);
        }
    })
}

function makeStructArrayArray() {
    let arr = ArrayTest.makeStructArrayArray("a", "b", 5, 2, 3)
    console.log("new array is " + arr)
    assertEQ(arr.length, 3)
    assertEQ(arr[0].length, 2)
    for (let i = 0; i < arr.length; i++) {
        for (let j = 0; j < arr[i].length; j++) {
            assertEQ(arr[i][j].a, "a")
            assertEQ(arr[i][j].b, "b")
            assertEQ(arr[i][j].c, 5)
        }
    }
}

function main() {
    console.log("run main ... ArrayTest ...")

    const suite = new ArkTestsuite("ArrayTest")
    suite.addTest("sumArray", sumArray)
    suite.addTest("getArrayValue", getArrayValue)
    suite.addTest("toStingArray", toStingArray)
    suite.addTest("makeIntArray", makeIntArray)
    suite.addTest("makeIntArray2", makeIntArray2)
    suite.addTest("makeEnumArray", makeEnumArray)
    suite.addTest("changeEnumArray", changeEnumArray)
    suite.addTest("makeRecordArray", makeRecordArray)
    suite.addTest("changeRecordArray", changeRecordArray)
    suite.addTest("makeStructArray", makeStructArray)
    suite.addTest("changeStructArray", changeStructArray)
    suite.addTest("fetchBinaryData", fetchBinaryData)
    suite.addTest("makeStructArrayArray", makeStructArrayArray)
    System.exit(suite.run())
}