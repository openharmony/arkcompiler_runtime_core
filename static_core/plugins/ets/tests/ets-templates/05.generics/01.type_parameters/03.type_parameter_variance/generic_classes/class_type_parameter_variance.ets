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
desc: Test of usage modifiers in and out, as well as type parameter without modifier. 
---*/

class X<in T1, out T2, T3> {
    readonly t2_obj: T2
    t3_obj: T3
    constructor(p: T2) {
        this.t2_obj = p
    }

    f1(p: T1) {}

    f2(): T2 { return this.t2_obj }

    f3(p: T3): T3 { return p }
}

function main(): void {
    let x: X<number, number, number> = new X<number, number, number>(1)
    assertEQ(x.f2(), 1)
    assertEQ(x.f3(1), 1)
}
