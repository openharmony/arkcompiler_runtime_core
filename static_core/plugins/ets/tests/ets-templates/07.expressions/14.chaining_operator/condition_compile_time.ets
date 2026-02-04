/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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
desc: |-
    If an expression preceding a chaining operator is known at compile time to always evaluate to a nullish
    value (undefined or null) or a non-nullish value at runtime, then a compile-time warning is issued.
---*/

class Example {
    member: int = 1
    method(): int {
        return 1
    }
}
let ex = new Example

type Foo = () => int
let func: Foo = (): int => { return 1 }

function foo_undefined(): undefined { return undefined }
let lambda_undefined = (): undefined => { return undefined}

function foo_null(): null { return null }
let lambda_null = (): null => { return null}

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

function main(): void {
    // test field access
    let example: Example | undefined = undefined
    arktest.assertEQ(example?.member, undefined)
    let example1: Example | null = null
    arktest.assertEQ(example1?.member, undefined)
    let example2: Example | null = ex
    arktest.assertEQ(example2?.member, 1)

    // test method call
    let example3: Example | undefined = undefined
    arktest.assertEQ(example3?.method(), undefined)
    let example4: Example | null = null
    arktest.assertEQ(example4?.method(), undefined)
    let example5: Example | null = ex
    arktest.assertEQ(example5?.method(), 1)

    // test function call
    let foo: Foo | undefined = undefined
    arktest.assertEQ(foo?.(), undefined)
    let foo1: Foo | null = null
    arktest.assertEQ(foo1?.(), undefined)
    let foo2: Foo | null = func
    arktest.assertEQ(foo2?.(), 1)

    // test indexing
    let array: string[] | undefined = undefined
    arktest.assertEQ(array?.[0], undefined)
    let array1: string[] | null = null
    arktest.assertEQ(array1?.[0], undefined)
    let array2: string[] | null = ["string"]
    arktest.assertEQ(array2?.[0], "string")
    test_undefined()
    test_null()
    test_undefined_lambda()
    test_null_lambda()
}