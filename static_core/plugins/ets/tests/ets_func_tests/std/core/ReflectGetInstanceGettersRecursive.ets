 /*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License')
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

class Simple {
    public a: int
    public b: int
}

class WithGetters {
    public a: int

    public get b(): int {
        return 5
    }
}

class WithSetters {
    public a: int

    public set b(val: int) {
        this.a = val
    }
}

class WithGettersAndSetters {
    public a: int

    public get b(): int {
        return 5
    }

    public set b(val: int) {
        this.a = val
    }

    public set c(val: int) {
        this.a = val
    }

    public get d(): int {
        return 5
    }
}

class Empty {}

class Private {
    public a: int

    private get b(): int {
        return 5
    }
}

class Protected {
    protected get a(): int {
        return 5
    }

    public b: int
}

class Strings {
    public a: string = ""

    public get b(): string {
        return ""
    }
}

class Undefined {
    public a: int | undefined

    public get b(): undefined {
        return undefined
    }
}

class Derived extends WithGetters {
    public c: int

    public get d(): int {
        return 5
    }

    public set r(x: int) {
        this.c = x
    }
}

class DerivedDerived extends Derived {
    public e: int

    public get f(): int {
        return 5
    }
}

interface Interface1 {
    a: int;
    foo1(): void;
    get getter1(): int;
}

interface Interface2 {
    b: int;
    foo2(): void;
    get getter2(): int;
}

class Impl1 implements Interface1 {
    a: int;
    c: int;
    
    constructor() {
        this.a = 0;
        this.c = 0;
    }
    
    foo1(): void {}

    get getter1(): int {
        return 5
    }

    get getterImpl1(): int {
        return 5
    }
}

class Impl2 implements Interface2 {
    b: int;
    c: int;
    
    constructor() {
        this.b = 0;
        this.c = 0;
    }
    
    foo2(): void {}

    get getter2(): int {
        return 5
    }

    get getterImpl2(): int {
        return 5
    }
}

class Impl12 implements Interface1, Interface2 {
    a: int;
    b: int;
    c: int;
    
    constructor() {
        this.a = 0;
        this.b = 0;
        this.c = 0;
    }
    
    foo1(): void {}
    foo2(): void {}


    get getter1(): int {
        return 5
    }

    get getter2(): int {
        return 5
    }
}

function check(cls: Class, names: Set<String>) {
    let getters = reflect.getInstanceGettersRecursive(cls)
    arktest.assertEQ(getters.length, names.size)
    for (let getter of getters) {
        arktest.assertTrue(getter.isGetter(), getter.getName() + " is not getter")
        arktest.assertTrue(names.has(getter.getName().substring(6)), cls.getName() + " has not " + getter.getName())
    }
}

function testBasic() {
    let simple = Class.from<Simple>()
    check(simple, new Set<String>([]))

    let withGetters = Class.from<WithGetters>()
    check(withGetters, new Set<String>(["b"]))

    let withSetters = Class.from<WithSetters>()
    check(withSetters, new Set<String>([]))

    let withGettersAndSetters = Class.from<WithGettersAndSetters>()
    check(withGettersAndSetters, new Set<String>(["b", "d"]))

    let empty = Class.from<Empty>()
    check(empty, new Set<String>([]))

    let priv = Class.from<Private>()
    check(priv, new Set<String>([]))

    let prot = Class.from<Protected>()
    check(prot, new Set<String>([]))

    let str = Class.from<Strings>()
    check(str, new Set<String>(["b"]))

    let undef = Class.from<Undefined>()
    check(undef, new Set<String>(["b"]))
}

function testDerived() {
    let der = Class.from<Derived>()
    check(der, new Set<String>(["b", "d"]))

    let derder = Class.from<DerivedDerived>()
    check(derder, new Set<String>(["b", "d", "f"]))
}

function testInterfaces() {
    let int1 = Class.from<Interface1>()
    check(int1, new Set<String>(["a", "getter1"]))

    let int2 = Class.from<Interface2>()
    check(int2, new Set<String>(["b", "getter2"]))

    let impl1 = Class.from<Impl1>()
    check(impl1, new Set<String>(["a", "getter1", "getterImpl1"]))

    let impl2 = Class.from<Impl2>()
    check(impl2, new Set<String>(["b", "getter2", "getterImpl2"]))

    let impl12 = Class.from<Impl12>()
    check(impl12, new Set<String>(["a", "getter1", "b", "getter2"]))
}

function main(): int {
    const suite = new arktest.ArkTestsuite("reflect.getInstanceGettersRecursive")

    suite.addTest("Test getter names in basic classes", testBasic)
    suite.addTest("Test field names in inherited classes", testDerived)
    suite.addTest("Test field names within interfaces", testInterfaces)

    return suite.run()
}
