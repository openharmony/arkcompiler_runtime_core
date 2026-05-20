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
    The forms using the keyword super are valid in instance methods, constructors of a class or
    in the initializer of an instance variable of a class. Compile-time error occurs otherwise.
---*/

class A0 {
    private readonly _x: int = 5

    get x(): int {
        return this._x
    }
}

class B0 extends A0 {
    public y: int = super.x
}

class A1 {
    private _x: int = 5

    protected get x(): int {
        return this._x
    }
}

class B1 extends A1 {
    private y: int = super.x

    foo(): int {
        return this.y
    }
}

class A2 {
    private _x: int = 5

    protected get x(): int {
        return this._x
    }
}

class B2 extends A2 {
    protected readonly y: int = super.x

    foo(): int {
        return this.y
    }
}

class A3 {
    private _x: String = new String("some text")

    protected get x(): String {
        return this._x
    }
}

class B3 extends A3 {
    protected readonly y: String = super.x

    foo(): String {
        return this.y
    }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("super.x in instance field initializer")

    suite.addTest("public field initialized from super getter (readonly source)", () => {
        let instance: B0 = new B0()
        arktest.assertEQ(instance.y, 5)
    })

    suite.addTest("private field initialized from protected super getter", () => {
        let instance: B1 = new B1()
        arktest.assertEQ(instance.foo(), 5)
    })

    suite.addTest("protected readonly field initialized from protected super getter", () => {
        let instance: B2 = new B2()
        arktest.assertEQ(instance.foo(), 5)
    })

    suite.addTest("protected readonly String field initialized from super getter", () => {
        let instance: B3 = new B3()
        arktest.assertEQ(instance.foo(), "some text")
    })

    return suite.run()
}
