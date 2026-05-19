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
    The forms using the keyword super are valid in instance methods, constructors of a class or
    in the initializer of an instance variable of a class. Compile-time error occurs otherwise.
---*/

class AFromBase {
    private _x: int

    get x(): int {
        return this._x
    }
}

final class BFromBase extends AFromBase {
    public bar(): int {
        return super.x
    }
}

class AFromCallStack {
    private _x: int

    get x(): int {
        return this._x
    }
}

final class BFromCallStack extends AFromCallStack {
    private foo(): int {
        return super.x
    }

    public bar(): int {
        return this.foo()
    }
}

class AFromSetterCtor {
    private _x: int

    protected get x(): int {
        return this._x
    }

    protected set x(p: int) {
        this._x = p
    }
}

class BFromSetterCtor extends AFromSetterCtor {
    constructor() {
        super.x = 7
    }

    public bar(): int {
        return this.x
    }
}

class AFromOverride {
    private _x: int

    get x(): int {
        return this._x
    }

    public bar(): int {
        return 1
    }
}

final class BFromOverride extends AFromOverride {
    public override bar(): int {
        return super.x
    }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("super.x access from instance method")

    suite.addTest("super.x returned directly from public method", () => {
        let instance: BFromBase = new BFromBase()
        arktest.assertEQ(instance.bar(), 0)
    })

    suite.addTest("super.x returned through private helper called by public method", () => {
        let instance: BFromCallStack = new BFromCallStack()
        arktest.assertEQ(instance.bar(), 0)
    })

    suite.addTest("super.x assigned in constructor through setter, read via this", () => {
        let instance: BFromSetterCtor = new BFromSetterCtor()
        arktest.assertEQ(instance.bar(), 7)
    })

    suite.addTest("override method returns super.x bypassing inherited body", () => {
        let instance = new BFromOverride
        arktest.assertEQ(instance.bar(), 0)
    })

    return suite.run()
}
