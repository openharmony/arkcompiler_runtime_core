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

function foo<T extends Object>(p: Object): string {
    if (p instanceof Array) {
        //  type erasure allows to check if parameter is of Array type
        return '' + (p as Array<T>)[0] + (p as Array<T>)[1]
    }
    if (p instanceof FixedArray<Object|undefined>) {
        //  type check of FixedArray<T> is available
        return '' + (p as FixedArray<T>)[1] + (p as FixedArray<T>)[2]
    }
    return ''
}

function main() {
    arktest.assertEQ(foo<string>(['a', 'b', 'c']), 'ab')
    arktest.assertEQ(foo<string>(['a', 'b', 'c'] as FixedArray<string>), 'bc')
    arktest.assertEQ(foo<string>(['a', 'b', 'c'] as [string, string, string]), '')
}
