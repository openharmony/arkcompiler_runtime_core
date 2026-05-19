/*---
Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
    The result of the field access expression is value or variable of the static field in the class or interface
    that is the type of the object reference expression. For a static field the result of an object reference
    expression evaluation is discarded. If the field is not readonly, then the result is variable.
---*/

class AStatic0 {
    static a: int;
    static { AStatic0.a = 1 }
}

class AStatic1 {
    static a: int;
    static { AStatic1.a = 1 }
}

class AStatic2 {
    static a: int;
    constructor() { AStatic2.a = 1 }
}

class AStatic3 {
    static a: int;
    constructor() { AStatic3.a = 1 }
}

class AStatic4 {
    static a: int;
    constructor() { AStatic4.a = 1 }
}

class AStatic5 {
    static a: int;
    a: int;
    constructor() {
        AStatic5.a = 1;
        this.a = 2;
    }
}

class AStatic6 {
    static a: int;
    static { AStatic6.a = 1 }
}

class AStatic7 {
    static a: int;
    constructor() { AStatic7.a = 1 }
}

class AStatic8 {
    static a: int;
    static { AStatic8.a = 2 }
    constructor() { AStatic8.a = 1 }
}

let mStatic4: int

function main(): int {
    let suite = new arktest.ArkTestsuite("Static field access via class type")

    suite.addTest("static initializer sets value, instance created", () => {
        let i: AStatic0 = new AStatic0()
        arktest.assertEQ(AStatic0.a, 1)
    })

    suite.addTest("static initializer sets value, no instance", () => {
        arktest.assertEQ(AStatic1.a, 1)
    })

    suite.addTest("constructor sets static value, instance created", () => {
        let i: AStatic2 = new AStatic2()
        arktest.assertEQ(AStatic2.a, 1)
    })

    suite.addTest("constructor sets static value, no instance — default zero", () => {
        arktest.assertTrue(AStatic3.a == 0)
    })

    suite.addTest("constructor sets static value, observed before and after new", () => {
        if (mStatic4 == 0) {
            arktest.assertTrue(AStatic4.a == 0)
            mStatic4++
        }
        let i: AStatic4 = new AStatic4()
        arktest.assertTrue(AStatic4.a == 1)
        arktest.assertTrue(mStatic4 == 1)
    })

    suite.addTest("static and instance fields coexist", () => {
        let i: AStatic5 = new AStatic5()
        arktest.assertTrue(AStatic5.a == 1)
        arktest.assertTrue(i.a == 2)
    })

    suite.addTest("static field reassigned after instantiation (static init)", () => {
        let i: AStatic6 = new AStatic6()
        AStatic6.a = 2
        arktest.assertTrue(AStatic6.a == 2)
    })

    suite.addTest("static field reassigned after instantiation (constructor)", () => {
        let i: AStatic7 = new AStatic7()
        AStatic7.a = 2
        arktest.assertTrue(AStatic7.a == 2)
    })

    suite.addTest("constructor overrides static initializer, then external reassignment", () => {
        let i: AStatic8 = new AStatic8()
        arktest.assertTrue(AStatic8.a == 1)
        AStatic8.a = 3
        arktest.assertTrue(AStatic8.a == 3)
    })

    return suite.run()
}
