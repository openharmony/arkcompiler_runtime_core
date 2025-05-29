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
desc: methods returning this, override return type is this, positive.
tags: []
---*/
class C {
    b: int
    foo(): this {
        this.b = 1
        return this
    }
}
class D extends C {
    foo(): this {
        this.b = 2
        return this
    }
}

function main(): void {
    let x: D = new D().foo()
    assertEQ(x.b, 2)
    let y: C = new C().foo()
    assertEQ(y.b, 1)    
    let z: C = new D()
    z.foo()
    assertEQ(z.b, 2)
}

