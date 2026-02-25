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
desc: >-
    Mandatory call to another same-class constructor that uses the keyword this.
    Mandatory call to a superconstructor if a class has an extension clause.
---*/

class Base {
    x: Object
    constructor() {
        this.x = new Object() // Base object is fully initialized
        crash_this(this)
    }
}

class Derived extends Base {
    y: Object
    constructor() {
        super() // mandatory call to base class constructor
        this.y = new Object()
    }
}

function crash_this(b: Base) {
    if (b instanceof Derived) {
        // If b is of type Derived, then
        arktest.assertTrue((b as Derived).y.toString() instanceof string) // Access y field of Derived object
        // Depending on the compilation context, either the compiler reports
        // a compile-time error, or the runtime system is to detect the case
    }
}

function main() {
    let b: Base = new Base()
    arktest.assertTrue(b.x.toString() instanceof string)

    arktest.expectThrow(
        () => { b = new Derived() },
        (e) => e instanceof NullPointerError
    )
}
