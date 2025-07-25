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

function main() {
    let suite = new ArkTestsuite("Array Invoke tests")
    suite.addTest("$_invokeArrayNumber", testInvokeArrayNumber)
    suite.addTest("$_invokeArrayStringOneValue", testInvokeArrayStringOneValue)
    suite.addTest("$_invokeArrayStringManyValues", testInvokeArrayStringManyValues)
    // NOTE(aleksander-sotov) enable test when internal issue 25291 is resolved
    //suite.addTest("$_invokeArrayNumberWithInt", testInvokeArrayNumberWithInt);
    return suite.run()
}

function testInvokeArrayNumber() {
    let arr = Array<number>(1.0, 2.0, 3.0, 4.0)
    for (let i: int = 0; i < arr.length; i++) {
        assertEQ(arr[i], (i + 1) as double)
    }
}

function testInvokeArrayStringOneValue() {
    let arr = Array<string>("abcd")
    assertEQ(arr[0], "abcd")
}

function testInvokeArrayStringManyValues() {
    let arr = Array<string>("abcd", "ef", "123")
    assertEQ(arr[0], "abcd")
    assertEQ(arr[1], "ef")
    assertEQ(arr[2], "123")
}

function testInvokeArrayNumberWithInt() {
    let arr = Array<number>(1, 2, 3)
    for (let i: int = 0; i < arr.length; i++) {
        assertEQ(arr[i], (i + 1) as double)
    }
}

