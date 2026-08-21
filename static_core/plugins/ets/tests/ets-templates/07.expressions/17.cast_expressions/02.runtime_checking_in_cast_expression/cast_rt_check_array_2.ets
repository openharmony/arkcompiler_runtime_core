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
    Test for array with specific types of elements.
---*/

function foo(p: Object): number {
    if (p instanceof Array) {
        //  type erasure allows to check if parameter is of Array type
        return (p as Array<number>)[0]
    }
    if (p instanceof FixedArray<number|undefined>) {
        //  type check of FixedArray<number|undefined> is available
        return (p as FixedArray<number>)[1]
    }
    if (p instanceof ValueArray<number>) {
        //  type check of ValueArray<number> is available
        return (p as ValueArray<number>)[2]
    }
    return 0
}

function main() {
    arktest.assertEQ(foo([1.0, 2.0, 3.0]), 1)
    arktest.assertEQ(foo([1, 2, 3] as FixedArray<number>), 2)
    arktest.assertEQ(foo([1, 2, 3] as ValueArray<number>), 3)
    arktest.assertEQ(foo([1, 2, 3] as [number, number, number]), 0)
}
