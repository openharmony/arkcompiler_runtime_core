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
desc: calling method returning 'this' from coroutine
---*/

import { launch } from "std/concurrency"

class A {
    fld: int = 0

    foo(): this {
        if (this.fld == 999) {
            throw new Error()
        }
        this.fld += 1
        return this
    }

    bar(): Promise<this> {
        let self = this.foo().foo()
        return launch<this>() {
            return self.foo()
        }
    }
}

async function main(): Promise<void> {
    let a = new A()

    arktest.assertEQ(await a.bar(), a)
    arktest.assertEQ(a.fld, 3)
}