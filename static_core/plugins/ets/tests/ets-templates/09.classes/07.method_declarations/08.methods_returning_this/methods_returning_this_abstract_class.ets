/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: methods returning this, declare class, positive.
---*/

abstract class C {
     foo(): this {
        return this
    }
} 

class D extends C {
}

class E extends C {
    foo(): this {
        return this
    }
}

function main(): void {
    let a: C = new D().foo()
    arktest.assertTrue(a instanceof C, "expect a instance of C")
    let b: D = new D().foo()
    arktest.assertTrue(b instanceof C, "expect b instance of C")

    let x: C = new E().foo()
    arktest.assertTrue(x instanceof C, "expect x instance of C")
    let y: E = new E().foo()
    arktest.assertTrue(y instanceof C, "expect y instance of C")
}
