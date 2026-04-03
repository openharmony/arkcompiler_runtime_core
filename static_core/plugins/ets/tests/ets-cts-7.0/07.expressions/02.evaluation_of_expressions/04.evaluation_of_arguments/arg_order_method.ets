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
    Argument list evaluation order for method calls progresses from
    left to right up to the first error or through the end of the expression.
---*/

let order: string = ""

function track(label: string, value: int): int {
    order += label
    return value
}

class Calculator {
    add(a: int, b: int, c: int): int {
        return a + b + c
    }

    multiply(a: int, b: int): int {
        return a * b
    }
}

class Chain {
    value: int = 0

    step(a: int, b: int): Chain {
        this.value += a + b
        return this
    }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Method argument evaluation order")
    let calc = new Calculator()

    suite.addTest("method args evaluated left to right", () => {
        order = ""
        let result = calc.add(track("A", 1), track("B", 2), track("C", 3))
        arktest.assertEQ(order, "ABC")
        arktest.assertEQ(result, 6)
    })

    suite.addTest("method args with two parameters left to right", () => {
        order = ""
        let result = calc.multiply(track("X", 5), track("Y", 3))
        arktest.assertEQ(order, "XY")
        arktest.assertEQ(result, 15)
    })

    suite.addTest("chained method calls evaluate args left to right", () => {
        order = ""
        let c = new Chain()
        c.step(track("A", 1), track("B", 2)).step(track("C", 3), track("D", 4))
        arktest.assertEQ(order, "ABCD")
        arktest.assertEQ(c.value, 10)
    })

    suite.addTest("side effects in method args follow left to right order", () => {
        let counter: int = 0
        let inc = (): int => { return ++counter }
        let result = calc.add(inc(), inc(), inc())
        arktest.assertEQ(result, 6)
        arktest.assertEQ(counter, 3)
    })

    return suite.run()
}
