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
    private b: int
}

class Protected {
    protected a: int
    public b: int
}

class Strings {
    public a: string = ""
    public b: int
    public c: string = ""
}

class Undefined {
    public a: int | undefined
    public b: undefined
}

class Derived extends Simple {
    public c: int
    public d: int
}

class DerivedDerived extends Derived {
    public e: int
    public f: int
}

class DerivedWithGetters extends WithGetters {
    public c: int
    public get d(): int {
        return 0
    }
}

interface Interface1 {
    a: int;
    foo1(): void;
}

interface Interface2 {
    b: int;
    foo2(): void;
}

class Impl1 implements Interface1 {
    a: int;
    c: int;
    
    constructor() {
        this.a = 0;
        this.c = 0;
    }
    
    foo1(): void {}
}

class Impl2 implements Interface2 {
    b: int;
    c: int;
    
    constructor() {
        this.b = 0;
        this.c = 0;
    }
    
    foo2(): void {}
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
}

function check(cls: Class, names: Set<String>) {
    let fields = reflect.getInstanceFieldsRecursive(cls)
    arktest.assertEQ(fields.length, names.size)
    for (let field of fields) {
        arktest.assertTrue(names.has(field.getName()), cls.getName() + " has not " + field.getName())
    }
}

function testBasic() {
    let simple = Class.from<Simple>()
    check(simple, new Set<String>(["a", "b"]))

    let empty = Class.from<Empty>()
    check(empty, new Set<String>([]))

    let priv = Class.from<Private>()
    check(priv, new Set<String>(["a"]))

    let prot = Class.from<Protected>()
    check(prot, new Set<String>(["b"]))

    let str = Class.from<Strings>()
    check(str, new Set<String>(["a", "b", "c"]))

    let undef = Class.from<Undefined>()
    check(undef, new Set<String>(["a", "b"]))
}

function testGettersSetters() {
    let withGetters = Class.from<WithGetters>()
    check(withGetters, new Set<String>(["a"]))

    let withSetters = Class.from<WithSetters>()
    check(withSetters, new Set<String>(["a"]))

    let withGettersAndSetters = Class.from<WithGettersAndSetters>()
    check(withGettersAndSetters, new Set<String>(["a"]))
}

function testDerived() {
    let der = Class.from<Derived>()
    check(der, new Set<String>(["a", "b", "c", "d"]))

    let derder = Class.from<DerivedDerived>()
    check(derder, new Set<String>(["a", "b", "c", "d", "e", "f"]))

    let derWithGetters = Class.from<DerivedWithGetters>()
    check(derWithGetters, new Set<String>(["a", "c"]))
}

function testInterfaces() {
    let int1 = Class.from<Interface1>()
    check(int1, new Set<String>([]))

    let int2 = Class.from<Interface2>()
    check(int2, new Set<String>([]))

    let impl1 = Class.from<Impl1>()
    check(impl1, new Set<String>(["c"]))

    let impl2 = Class.from<Impl2>()
    check(impl2, new Set<String>(["c"]))

    let impl12 = Class.from<Impl12>()
    check(impl12, new Set<String>(["c"]))
}

function main(): int {
    const suite = new arktest.ArkTestsuite("reflect.getInstanceFieldsRecursive")

    suite.addTest("Test field names in basic classes", testBasic)
    suite.addTest("Test field names in classes with getters and/or setters", testGettersSetters)
    suite.addTest("Test field names in inherited classes", testDerived)
    suite.addTest("Test field names within interfaces", testInterfaces)

    return suite.run()
}
