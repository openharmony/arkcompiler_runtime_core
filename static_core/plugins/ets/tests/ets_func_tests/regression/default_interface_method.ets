/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
desc: Default interface method call should not lead to memory leaks
tags: []
---*/

interface I {
    // abstract method
    foo(p: Int): Int;

    // default method (reference types are important)
    bar(p: Int): Int {
        return p * 4;
    }
}

class C implements I {
    fld: int;

    // overridden interface method
    public override foo(p: Int): Int {
        this.fld = p * 3;
        return this.fld;
    }
}

function main(): void {
    let c: C = new C();

    assert c.foo(2) == 6
    assert c.bar(2) == 8
}
