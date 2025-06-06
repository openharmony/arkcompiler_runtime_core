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

function main(): void {
    const arr = new Array<number>()
    assertEQ( arr.length, 0)
    for (let i = 0; i < 30; i++) {
        assertEQ( arr.length, i)
        arr.push(i as number)
    }
    for (let i = 0; i < 30; i++) {
        assertTrue( arr.includes(i as number, undefined))
    }
    assertEQ( arr.at(50), undefined)
    assertEQ( arr.length, 30)
    for (let i = 0; i < 30; i++) {
        assertEQ( arr.pop()! as Object as Double as double as int, 30 - 1 - i)
    }
    assertEQ( arr.length, 0)

    test_reverse()
    test_splice_1()
    test_unshift()
}

function test_reverse(): void {
    const arr = new Array<number>()
    assertEQ( arr.length, 0)
    for (let i = 0; i < 30; i++) {
        assertEQ( arr.length, i)
        arr.push(i as number)
    }
    arr.reverse()
    for (let i = 0; i < 30; i++) {
        assertEQ( arr.indexOf(i as number, undefined), 30 - 1 - i)
    }
}

function test_splice_1(): void {
    const arr = new Array<number>()
    for (let i = 0; i <= 3; i++) {
        arr.push(i as number)
    }
    const other = arr.splice(1)
    assertEQ( arr.length, 1)
    assertEQ( arr.indexOf(0 as number, undefined), 0)

    assertEQ( other.length, 3)
    for (let i = 1; i <= 3; i++) {
        assertEQ( other.indexOf(i as number, undefined), i - 1)
    }

    other.splice(1, undefined, 6, 7, 8);
    assertEQ( other.length, 6)
    assertEQ( other.$_get(0), 1)
    assertEQ( other.$_get(1), 6)
    assertEQ( other.$_get(2), 7)
    assertEQ( other.$_get(3), 8)
    assertEQ( other.$_get(4), 2)
    assertEQ( other.$_get(5), 3)
}

function test_unshift(): void {
    const a = new Array<number>()
    a.unshift(1, 2, 3)
    assertEQ( a.length, 3)
    a.unshift(4, 5, 6)
    a.unshift(7, 8, 9)
    assertEQ( a.length, 9)
    assertEQ( a.$_get(0), 7)
    assertEQ( a.$_get(1), 8)
    assertEQ( a.$_get(2), 9)
    assertEQ( a.$_get(3), 4)
    assertEQ( a.$_get(4), 5)
    assertEQ( a.$_get(5), 6)
    assertEQ( a.$_get(6), 1)
    assertEQ( a.$_get(7), 2)
    assertEQ( a.$_get(8), 3)
}
