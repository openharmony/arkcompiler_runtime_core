/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: several methods returning this, call several times, positive.
tags: []
---*/
class C {
    b: int = 1
    b2: int = 1
    foo(): this {
        this.b++
        return this
    }
    foo2(): this {
        this.b2++
        return this
    }
}

function main(): void {
    let x: C = new C()
    x=x.foo().foo2().foo().foo2().foo().foo2().foo().foo2().foo().foo2().foo2().foo2().foo().foo().foo()
    assertEQ(x.b, 9)
    assertEQ(x.b2, 8)
}
