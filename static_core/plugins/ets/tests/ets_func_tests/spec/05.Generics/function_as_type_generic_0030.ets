/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc: 05.Generics
tags: []
---*/

type func_int_int = (p: int) => int

function foo() :  Array<func_int_int>{
    const a = new Array<func_int_int>
    return a;
}


function main() {
    let a = foo();
    a.push((p) => p + 1 );
    a.push((p) => p + 2 );
    a.push((p) => p + 3 );
    a.push((p) => p + 4 );
    assertEQ(a[0](1), 2)
    assertEQ(a[0](1), 3)
    assertEQ(a[0](1), 4)
    assertEQ(a[0](1), 5)
    return 0;
}


