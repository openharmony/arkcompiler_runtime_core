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

class ACtorPublic {
    public x: int

    public constructor(x: int) {
        this.x = x
    }
}

class BCtorPublic extends ACtorPublic {
    public constructor() {
        super(5)
    }
}

class ACtorProtected {
    public x: int

    protected constructor(x: int) {
        this.x = x
    }
}

class BCtorProtected extends ACtorProtected {
    public constructor() {
        super(5)
    }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("super(...) call from subclass constructor")

    suite.addTest("subclass constructor calls public super constructor", () => {
        let instance: BCtorPublic = new BCtorPublic()
        arktest.assertEQ(instance.x, 5)
    })

    suite.addTest("subclass constructor calls protected super constructor", () => {
        let instance: BCtorProtected = new BCtorProtected()
        arktest.assertEQ(instance.x, 5)
    })

    return suite.run()
}
