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
desc: NonNullable leaves T intact if T contains neither null nor undefined
---*/

class C {
    v: number = 42
}

function main() {
    // NonNullable<T> where T has no null/undefined is same as T
    let snn: NonNullable<string> = "hello"
    let s: string = "world"
    snn = s
    s = snn

    let nnn: NonNullable<number> = 42
    let n: number = 24
    nnn = n
    n = nnn

    let onn: NonNullable<Object> = new Object
    let o: Object = new Object
    onn = o
    o = onn

    let cnn: NonNullable<C> = new C
    let c: C = new C
    cnn = c
    c = cnn
}
