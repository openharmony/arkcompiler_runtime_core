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
desc: >-
    The result of a field access expression of an instance accessor in a class
    is a call to a proper getter if the expression is not assigned a new
    expression, or a call to a proper setter if the expression is in the
    left-hand-side of the assignment.
---*/

class Counter {
    private _count: int = 0
    private _getCalls: int = 0
    private _setCalls: int = 0

    get count(): int {
        this._getCalls++
        return this._count
    }

    set count(value: int) {
        this._setCalls++
        this._count = value
    }

    getGetCalls(): int {
        return this._getCalls
    }

    getSetCalls(): int {
        return this._setCalls
    }

    getRawCount(): int {
        return this._count
    }
}

class ReadOnlyBox {
    private _value: int

    constructor(v: int) {
        this._value = v
    }

    get value(): int {
        return this._value
    }
}

interface HasName {
    get name(): string
}

class Person implements HasName {
    private _name: string

    constructor(n: string) {
        this._name = n
    }

    get name(): string {
        return this._name
    }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Accessor field access expressions")

    suite.addTest("getter is called when reading accessor property", () => {
        let c = new Counter()
        let val = c.count
        arktest.assertEQ(val, 0)
        arktest.assertEQ(c.getGetCalls(), 1)
    })

    suite.addTest("setter is called when assigning to accessor property", () => {
        let c = new Counter()
        c.count = 10
        arktest.assertEQ(c.getRawCount(), 10)
        arktest.assertEQ(c.getSetCalls(), 1)
    })

    suite.addTest("getter and setter calls are tracked independently", () => {
        let c = new Counter()
        c.count = 5
        let v1 = c.count
        c.count = 8
        let v2 = c.count
        arktest.assertEQ(v1, 5)
        arktest.assertEQ(v2, 8)
        arktest.assertEQ(c.getGetCalls(), 2)
        arktest.assertEQ(c.getSetCalls(), 2)
    })

    suite.addTest("read-only accessor returns correct value", () => {
        let b = new ReadOnlyBox(99)
        arktest.assertEQ(b.value, 99)
    })

    suite.addTest("interface property accessor via class instance", () => {
        let p: HasName = new Person("Alice")
        arktest.assertEQ(p.name, "Alice")
    })

    suite.addTest("compound assignment calls both getter and setter", () => {
        let c = new Counter()
        c.count = 3
        c.count += 7
        arktest.assertEQ(c.getRawCount(), 10)
        arktest.assertEQ(c.getGetCalls(), 1)
        arktest.assertEQ(c.getSetCalls(), 2)
    })

    return suite.run()
}
