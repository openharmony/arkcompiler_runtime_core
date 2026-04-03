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
    Argument list evaluation order for constructor calls progresses from
    left to right up to the first error or through the end of the expression.
---*/

let order: string = ""

function track(label: string, value: int): int {
    order += label
    return value
}

class Point {
    x: int
    y: int
    constructor(x: int, y: int) {
        this.x = x
        this.y = y
    }
}

class Triple {
    a: int
    b: int
    c: int
    constructor(a: int, b: int, c: int) {
        this.a = a
        this.b = b
        this.c = c
    }
}

class Nested {
    inner: Point
    extra: int
    constructor(p: Point, extra: int) {
        this.inner = p
        this.extra = extra
    }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Constructor argument evaluation order")

    suite.addTest("constructor args with two parameters left to right", () => {
        order = ""
        let p = new Point(track("X", 10), track("Y", 20))
        arktest.assertEQ(order, "XY")
        arktest.assertEQ(p.x, 10)
        arktest.assertEQ(p.y, 20)
    })

    suite.addTest("constructor args with three parameters left to right", () => {
        order = ""
        let t = new Triple(track("A", 1), track("B", 2), track("C", 3))
        arktest.assertEQ(order, "ABC")
        arktest.assertEQ(t.a, 1)
        arktest.assertEQ(t.b, 2)
        arktest.assertEQ(t.c, 3)
    })

    suite.addTest("side effects in constructor args follow left to right order", () => {
        let counter: int = 0
        let inc = (): int => { return ++counter }
        let t = new Triple(inc(), inc(), inc())
        arktest.assertEQ(t.a, 1)
        arktest.assertEQ(t.b, 2)
        arktest.assertEQ(t.c, 3)
        arktest.assertEQ(counter, 3)
    })

    suite.addTest("nested constructor arg evaluation order", () => {
        order = ""
        let n = new Nested(new Point(track("A", 5), track("B", 6)), track("C", 7))
        arktest.assertEQ(order, "ABC")
        arktest.assertEQ(n.inner.x, 5)
        arktest.assertEQ(n.inner.y, 6)
        arktest.assertEQ(n.extra, 7)
    })

    return suite.run()
}
