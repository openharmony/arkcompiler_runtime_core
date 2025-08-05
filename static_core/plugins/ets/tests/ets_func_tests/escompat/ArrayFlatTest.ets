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

function compareString(first: string, second: string) {
    arktest.assertEQ(first, second)
}

function compareArray<T>(first: Array<T>, second: Array<T>) {
    arktest.assertEQ(first.length, second.length)
    for(let i = 0; i < first.length; i++) {
        if(first[i] instanceof Array) {
            compareArray<T>(first[i] as Array<T>, second[i] as Array<T>)
        } else {
            arktest.assertEQ(first[i], second[i])
        }
    }
}

function booleanTest() {
    const arr = new Array<boolean | Array<boolean>>(true, [false, true], false)
    compareArray<boolean>(arr.flat<boolean>(), [true,false,true,false])
}

function intTest() {
    const arr = [0, 1, [2, [3, [4, 5]]]]
    const arr1 = [1, [1, [1, undefined ,2], 2], 3]
    const arr2 = new Array<Number | Array<Array<Number> | Number>>(1, 2, 3, [4, [5, 6]])

    compareArray<int | Array<int | Array<int>>>(arr.flat<int | Array<int | Array<int>>>(), [0,1,2,[3,[4,5]]])
    compareArray<int | Array<int>>(arr.flat<int | Array<int>>(2), [0,1,2,3,[4,5]])
    compareArray<int>(arr.flat<int>(Infinity), [0,1,2,3,4,5])
    compareArray<int | Array<int | Array<int | Array<int>>>>(arr.flat<int | Array<int | Array<int | Array<int>>>>(-Infinity), [0,1,[2,[3,[4,5]]]])
    compareArray<int | Array<int | Array<int | Array<int>>>>(arr.flat<int | Array<int | Array<int | Array<int>>>>(NaN), [0,1,[2,[3,[4,5]]]])
    compareArray<int | undefined>(arr1.flat<int | undefined>(Infinity), [1,1,1,undefined,2,2,3])
    compareArray<int | Array<int>>(arr2.flat<int | Array<int>>(), [1,2,3,4,[5,6]])
}

function stringTest() {
    const arr = ["1", "2", ["3", ["4"], "5"], "6"]
    const arr2 = new Array<String | Number | Array<bigint>>("1", 2, [3n])
    compareArray<String | Array<String>>(arr.flat<String | Array<String>>(), ["1", "2", "3", ["4"], "5", "6"])
    compareArray<String | Number | bigint>(arr2.flat<String | Number | bigint>(), ["1", 2, 3n])
}

function flatMapTest(){
    const arr = [1, 2, 1]
    const result = arr.flatMap<int | Array<int | Array<int>>>((num) => (num === 2 ? [[2, 2], 2] : 1))
    compareArray<int | Array<int | Array<int>>>(result, [1,[2,2],2,1] as Array<int | Array<int | Array<int>>>)

    const arr1 = [1, undefined, 1];
    const result1 = arr1.flatMap<int | Array<int>>((num) => (num === 2 ? [2, 2] : 1));
    compareArray<int | Array<int>>(result1, [1,1,1] as Array<int | Array<int>>)
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Array.flat tests')
    suite.addTest('for boolean', booleanTest)
    suite.addTest('for int', intTest)
    suite.addTest('for string', stringTest)
    suite.addTest('for flatMap', flatMapTest)
    return suite.run()
}