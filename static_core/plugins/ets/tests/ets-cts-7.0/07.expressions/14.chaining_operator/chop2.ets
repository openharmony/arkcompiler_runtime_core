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

function foo_undefined(): undefined { return undefined }
let lambda_undefined = (): undefined => { return undefined}

function foo_null(): null { return null }
let lambda_null = (): null => { return null}

function test_undefined() {
    let cond: boolean = Math.random() > 0.5
    let aa = new Example
    let a1: Example | undefined = cond ? aa : foo_undefined()
    let a2: Example | undefined = cond ? foo_undefined() : aa
    let res1: int|undefined = a1?.field
    let res2: int|undefined = a2?.field
    arktest.assertTrue((res1 == 1) || (res2 == 1), "expect 1 (u, field)")
    arktest.assertTrue((res1 == undefined) || (res2 == undefined), "expect undefined (u, field)")
    let res3: int|undefined = a1?.method()
    let res4: int|undefined = a2?.method()
    arktest.assertTrue((res3 == 1) || (res4 == 1), "expect 1 (u, method)")
    arktest.assertTrue((res3 == undefined) || (res4 == undefined), "expect undefined (u, method)")
}

function test_null() {
    let cond: boolean = Math.random() > 0.5
    let aa = new Example
    let a1: Example | null = cond ? aa : foo_null()
    let a2: Example | null = cond ? foo_null() : aa
    let res1: int|undefined = a1?.field
    let res2: int|undefined = a2?.field
    arktest.assertTrue((res1 == 1) || (res2 == 1), "expect 1 (n, field)")
    arktest.assertTrue((res1 == undefined) || (res2 == undefined), "expect undefined (n, field)")
    let res3: int|undefined = a1?.method()
    let res4: int|undefined = a2?.method()
    arktest.assertTrue((res3 == 1) || (res4 == 1), "expect 1 (n, method)")
    arktest.assertTrue((res3 == undefined) || (res4 == undefined), "expect undefined (n, method)")
}

function test_undefined_lambda() {
    let cond: boolean = Math.random() > 0.5
    let aa = new Example
    let a1: Example | undefined = cond ? aa : lambda_undefined()
    let a2: Example | undefined = cond ? lambda_undefined() : aa
    let res1: int|undefined = a1?.field
    let res2: int|undefined = a2?.field
    arktest.assertTrue((res1 == 1) || (res2 == 1), "expect 1 (u, field, lambda)")
    arktest.assertTrue((res1 == undefined) || (res2 == undefined), "expect undefined (u, field, lambda)")
    let res3: int|undefined = a1?.method()
    let res4: int|undefined = a2?.method()
    arktest.assertTrue((res3 == 1) || (res4 == 1), "expect 1 (u, method, lambda)")
    arktest.assertTrue((res3 == undefined) || (res4 == undefined), "expect undefined (u, method, lambda)")
}

function test_null_lambda() {
    let cond: boolean = Math.random() > 0.5
    let aa = new Example
    let a1: Example | null = cond ? aa : lambda_null()
    let a2: Example | null = cond ? lambda_null() : aa
    let res1: int|undefined = a1?.field
    let res2: int|undefined = a2?.field
    arktest.assertTrue((res1 == 1) || (res2 == 1), "expect 1 (n, field, lambda)")
    arktest.assertTrue((res1 == undefined) || (res2 == undefined), "expect undefined (n, field, lambda)")
    let res3: int|undefined = a1?.method()
    let res4: int|undefined = a2?.method()
    arktest.assertTrue((res3 == 1) || (res4 == 1), "expect 1 (n, method, lambda)")
    arktest.assertTrue((res3 == undefined) || (res4 == undefined), "expect undefined (n, method, lambda)")
}

function main() {
    test_undefined()
    test_null()
    test_undefined_lambda()
    test_null_lambda()
}
