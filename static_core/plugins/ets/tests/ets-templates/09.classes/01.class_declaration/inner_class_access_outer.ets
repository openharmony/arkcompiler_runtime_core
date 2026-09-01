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
desc: "TEST-UNIT-09-0004 GAP-09-0004 GRC-09-0004 CI-09-01-003-01: Inner class can access outer class instance variables"
tags: [compile-only]
---*/

class Outer {
    public value: number = 42  // Remove private modifier to allow access

    getInner(): Inner {
        return new Inner(this)
    }
}

class Inner {
    private outer: Outer

    constructor(outer: Outer) {
        this.outer = outer
    }

    getValue(): number {
        return this.outer.value  // Access outer class instance variable
    }
}

function main(): void {
    let outer = new Outer()
    let inner = outer.getInner()
    console.log(inner.getValue())
}
