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
desc: 09.9 Constructor Declaration
tags: [compile-only, negative]
---*/


class X {
    data : number;
    data1 : string
    data2 : long
    constructor (x:number, y:string = "Hello World", z: long = 1024) {
        this.data = x
        this.data1 = y
        this.data2 = z
    }
}

function main() : int {
    let x : X = new X("Bye World");
    assertEQ( x.data, 0)
    assertEQ( x.data1, "Bye World")
    assertEQ( x.data2, 1024)
    return 0;
}
