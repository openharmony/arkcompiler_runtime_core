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
desc: methods returning this, implements multiple interfaces with the different methods that return this, positive.
tags: []
---*/
interface I {
    foo1(): this {
        return this
    }
}

interface II {
    foo2(): this {
        return this
    }
}

class C implements I, II {
}

function main(): void {
    let x: C = new C()
    x = x.foo2()
    x = x.foo1()
    let y: C = new C()
    y = y.foo1()
    y = y.foo2()
}

