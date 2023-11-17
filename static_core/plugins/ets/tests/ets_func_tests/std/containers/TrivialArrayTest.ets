/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
    assert arr.length == 0
    for (let i = 0; i < 30; i++) {
        assert arr.length == i
        arr.push(i as number)
    }
    for (let i = 0; i < 30; i++) {
        assert arr.includes(i as number)
    }
    assert arr.at(50) == null
    assert arr.length == 30
    for (let i = 0; i < 30; i++) {
        assert arr.pop()! as Object as Double as double as int == 30 - 1 - i
    }
    assert arr.length == 0

    test_reverse()
    test_splice_1()
}

function test_reverse(): void {
    const arr = new Array<number>()
    assert arr.length == 0
    for (let i = 0; i < 30; i++) {
        assert arr.length == i
        arr.push(i as number)
    }
    arr.reverse()
    for (let i = 0; i < 30; i++) {
        assert arr.indexOf(i as number) == 30 - 1 - i
    }
}

function test_splice_1(): void {
    const arr = new Array<number>()
    for (let i = 0; i <= 3; i++) {
        arr.push(i as number)
    }
    const other = arr.splice(1)
    assert arr.length == 1
    assert arr.indexOf(0 as number) == 0

    assert other.length == 3
    for (let i = 1; i <= 3; i++) {
        assert other.indexOf(i as number) == i - 1
    }
}
