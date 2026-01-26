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

class A {
    name: string = "a"
}

function test(a: A | null | undefined): string | undefined {
    return a?.name
}

function main() {
    let a = new A
    let res1 = test(a)
    arktest.assertTrue(res1 == "a", "expect string")
    let res2 = test(undefined)
    arktest.assertTrue(res2 == undefined, "expect undefined (undefined)")
    let res3 = test(null)
    arktest.assertTrue(res3 == undefined, "expect undefined (null)")
}
