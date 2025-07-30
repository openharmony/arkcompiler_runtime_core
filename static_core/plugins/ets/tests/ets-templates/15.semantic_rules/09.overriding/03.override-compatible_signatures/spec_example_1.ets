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
desc: Class/Interface Types
---*/

interface Base {
    param(p: Derived): void
    ret(): Base
}

interface Derived extends Base {
    param(p: Base): void // Contravariant parameter
    ret(): Derived // Covariant return type
}

class B implements Base {
    param(p: Derived): void {
        res = 1
    }
    ret(): Base {
        return this
    }
}

class D implements Derived {
    param(p: Base): void {
        res = 2
    }
    ret(): Derived {
        return this
    }
}

let b = new B
let d = new D
let res = 0

function main() {
    b.param(d)
    arktest.assertEQ(res, 1)
    arktest.assertEQ(b.ret(), b)

    d.param(b)
    arktest.assertEQ(res, 2)
    arktest.assertEQ(d.ret(), d)
}
