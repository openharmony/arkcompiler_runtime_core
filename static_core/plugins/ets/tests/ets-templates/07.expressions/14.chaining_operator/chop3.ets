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
desc: >-
    If the object to the left of ('?.') is undefined or null, then the evaluation of the entire
    surrounding expression is dropped, and undefined is used as the result of the expression.
---*/

class Example {
    field: int = 1

    method(): int {
        return 1
    }
}

function foo_undefined(): Example | undefined { return undefined }
let lambda_undefined = (): Example | undefined => { return undefined}

function foo_null(): Example | null { return null }
let lambda_null = (): Example | null => { return null}

function test_undefined() {
    let a1: Example | undefined = foo_undefined()
    let res1: int|undefined = a1?.field
    arktest.assertTrue(res1 == undefined, "expect undefined (u, field)")
    let res3: int|undefined = a1?.method()
    arktest.assertTrue(res3 == undefined, "expect undefined (u, method)")
}

function test_null() {
    let a1: Example | null = foo_null()
    let res1: int|undefined = a1?.field
    arktest.assertTrue(res1 == undefined, "expect undefined (n, field)")
    let res3: int|undefined = a1?.method()
    arktest.assertTrue(res3 == undefined, "expect undefined (n, method)")
}

function test_undefined_lambda() {
    let a1: Example | undefined = lambda_undefined()
    let res1: int|undefined = a1?.field
    arktest.assertTrue(res1 == undefined, "expect undefined (u, field, lambda)")
    let res3: int|undefined = a1?.method()
    arktest.assertTrue(res3 == undefined, "expect undefined (u, method, lambda)")
}

function test_null_lambda() {
    let a1: Example | null = lambda_null()
    let res1: int|undefined = a1?.field
    arktest.assertTrue(res1 == undefined, "expect undefined (n, field, lambda)")
    let res3: int|undefined = a1?.method()
    arktest.assertTrue(res3 == undefined, "expect undefined (n, method, lambda)")
}

function main() {
    test_undefined()
    test_null()
    test_undefined_lambda()
    test_null_lambda()
}
