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

function foo() :  Array<(args: int) => int>{
    const a = new Array<(args: int) => int>
    return a;
}


function main() {
    let a = foo();
    let b1 = (p) => p + 1 
    a.push(b1)
    let b2 = (p) => p + 2 
    a.push(b2)
    let b3 = (p) => p + 3 
    a.push(b3)
    let b4 = (p) => p + 4 
    a.push(b4)
    assertEQ(a[0](1), 2)
    assertEQ(a[0](1), 3)
    assertEQ(a[0](1), 4)
    assertEQ(a[0](1), 5)
    return 0;
}


