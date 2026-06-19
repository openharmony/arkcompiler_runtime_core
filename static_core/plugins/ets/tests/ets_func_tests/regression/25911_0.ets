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
desc: Issue 25911. Error using FixedArray in Union type
tags: []
---*/

type MyAliasType1 = FixedArray<Int8Array>
type MyAliasType2 = Array<Int16Array>

function foo(t: MyAliasType1 | MyAliasType2) {
    let fold = 0
    for(let i of t) { fold += i.length }
    return fold
}

function main() {
    arktest.assertEQ(3, foo([new Int8Array([1, 2, 3])]))
}
