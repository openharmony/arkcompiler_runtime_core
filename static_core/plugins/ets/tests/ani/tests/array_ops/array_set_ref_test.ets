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

const string1 = 'New String 1!'
const string2 = 'New String 2!'
const helloWorld = 'Hello World!'

function getArray(): (string | null)[] {
    return [null, null, helloWorld]
}

function getEscompatArray(): Array<string | null> {
    let a: Array<string | null> = new Array<string | null>(3)
    a[0] = null
    a[1] = null
    a[2] = helloWorld
    return a
}

function getNewString1(): string {
    return string1
}

function getNewString2(): string {
    return string2
}

function checkArray(array: (string | null)[]): boolean {
    return array[0] == string1 && array[1] == null && array[2] == string2
}

function checkEscompatArray(array: Array<string | null>): boolean {
    return array[0] == string1 && array[1] == null && array[2] == string2
}

function getObj(): Object {
    return new Object()
}
