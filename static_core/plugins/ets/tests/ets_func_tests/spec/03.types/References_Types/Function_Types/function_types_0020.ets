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
 desc: 3.8.7 Function_Types
 tags: [compile-only]
 ---*/

function foo() : Array<(p1: Int, p2: Int) => int> {
    const listeners = new Array<(p1: Int, p2: Int) => int>()
    return listeners
}

function main () : int {
    let array = foo()
    let l = (x:Int, y:Int) => {return x + y}
    for (let i = 0; i < 6; i++ ) {
        array.push(l)
    }

    for (let i = 0; i < 6; i++) {
        assertEQ( array[i](i,i), i*2)
    }

    return 0;
}
