/*---
Copyright (c) 2021-2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: >-
  Any expression can be used as a statement.
  The execution of a statement leads to the execution of the expression.
  The result of such execution is discarded.
---*/

let sideEffect: int = 0

function increment(): int {
    sideEffect++
    return sideEffect
}

class Counter {
    count: int = 0
    inc(): Counter {
        this.count++
        return this
    }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Expression Statements")

    suite.addTest("String literal as expression statement", () => {
        "asd";
    })

    suite.addTest("Arithmetic literal as expression statement", () => {
        1 + 2;
    })

    suite.addTest("Array literal as expression statement", () => {
        ["hello", "world"];
    })

    suite.addTest("Arithmetic expression as statement discards result", () => {
        let x = 10
        x + 5;
        arktest.assertEQ(x, 10)
    })

    suite.addTest("Assignment expression as statement", () => {
        let x: int = 0
        x = 35;
        arktest.assertEQ(x, 35)
    })

    suite.addTest("Function call as expression statement with side effect", () => {
        sideEffect = 0
        increment();
        arktest.assertEQ(sideEffect, 1)
    })

    suite.addTest("Increment expression as statement", () => {
        let x = 5
        x++;
        arktest.assertEQ(x, 6)
    })

    suite.addTest("Decrement expression as statement", () => {
        let x = 5
        x--;
        arktest.assertEQ(x, 4)
    })

    suite.addTest("Method call as expression statement", () => {
        let c = new Counter()
        c.inc();
        arktest.assertEQ(c.count, 1)
    })

    suite.addTest("Chained method call as expression statement", () => {
        let c = new Counter()
        c.inc().inc().inc();
        arktest.assertEQ(c.count, 3)
    })

    suite.addTest("String concatenation as expression statement discards result", () => {
        let s = "hello"
        s + " world";
        arktest.assertEQ(s, "hello")
    })

    suite.addTest("Compound assignment as expression statement", () => {
        let x = 10
        x += 5;
        arktest.assertEQ(x, 15)
    })

    suite.addTest("Conditional expression as statement discards result", () => {
        let x = 10
        let y = 20
        x > y ? x : y;
        arktest.assertEQ(x, 10)
        arktest.assertEQ(y, 20)
    })

    suite.addTest("New expression as statement", () => {
        let c = new Counter();
        arktest.assertEQ(c.count, 0)
    })

    return suite.run()
}
