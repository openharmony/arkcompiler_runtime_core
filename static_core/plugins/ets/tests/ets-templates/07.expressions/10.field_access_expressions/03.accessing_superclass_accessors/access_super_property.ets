/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
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
    The forms using the keyword super are valid in instance methods, constructors of a class or
    in the initializer of an instance variable of a class. Compile-time error occurs otherwise.
---*/

class AAA0 {
    private readonly _x: int = 5
    get x(): int {
        Counter0.cc++
        return this._x
    }
}

final class BBB0 extends AAA0 {
    override get x(): int {
        Counter0.cc++
        return super.x
    }
}

final class Counter0 {
    static cc: int = 0
}

final class Counter1 {
    static cc: int = 0
}

class AAA1 {
    private readonly _x: int = 5
    get x(): int {
        Counter1.cc++
        return this._x
    }
}

final class BBB1 extends AAA1 {
    override get x(): int {
        Counter1.cc++
        return super.x
    }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("super.x access through overridden getter")

    suite.addTest("counter declared after subclass — overridden getter delegates to super and runs both", () => {
        Counter0.cc = 0
        let instance = new BBB0
        arktest.assertEQ(5, instance.x)
        arktest.assertEQ(2, Counter0.cc)
    })

    suite.addTest("counter declared before superclass — overridden getter delegates to super and runs both", () => {
        Counter1.cc = 0
        let instance = new BBB1
        arktest.assertEQ(5, instance.x)
        arktest.assertEQ(2, Counter1.cc)
    })

    return suite.run()
}
