/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

interface Iface {
    someDefaultMethod(): string {
        return "111"
    }
}

class A implements Iface {
    public static stfooFld1: Number = 1
    public static stfooFld2: Number = 1

    public fooFld1: Number = 1
    public fooFld2: Number = 1

    public foo1(): void {}
    public foo2(): void {}

    public static stfoo1(): void {}
    public static stfoo2(): void {}

    private prfoo1(): void {}
    private prfoo2(): void {}
}

final class B extends A {
    public static stbooFld1: Number = 2
    public static stbooFld2: Number = 2

    public booFld1: Number = 2
    public booFld2: Number = 2

    public boo1(): string {
        return ""
    }

    public boo2(): string {
        return "222"
    }

    public static stboo1(): string {
        return ""
    }

    public static stboo2(): string {
        return "222"
    }
}

class CtorsTest {
    foo: Number
    bla: String
    static numOfCtors: Int = 2

    constructor(x: Number) {
        this.foo = x;
        this.bla = "bla"
    }

    constructor(x: String) {
        this.bla = x;
        this.foo = 2;
    }
}

function testGetName(type: Class, expected: string) {
    arktest.assertEQ(type.getName(), expected)
}

function testGetInstanceMethods(classType: Class, expectedMethodsArr: string[]) {
    let expectedMtdsSet: Set<string> = new Set<string>(expectedMethodsArr)

    let methods: FixedArray<reflect.InstanceMethod> = classType.getInstanceMethods()
    arktest.assertEQ(methods.length, expectedMethodsArr.length)

    for (let i = 0; i < methods.length; ++i) {
        arktest.assertTrue(expectedMtdsSet.has(methods[i].getName()))
    }
}

function testGetInstanceMethodByName(classType: Class, methodsToFind: string[]) {
    for (let i = 0; i < methodsToFind.length; ++i) {
        let obtainedMethod = classType.getInstanceMethodByName(methodsToFind[i])

        if (obtainedMethod == undefined) {
            throw new Error(`Can't find given method '${methodsToFind[i]}' in testGetInstanceMethodByName`)
        }

        arktest.assertEQ(obtainedMethod.getName(), methodsToFind[i])
    }
}

function tryGetPrivateInstanceMethodByName(classType: Class, methods: string[]) {
    for (let i = 0; i < methods.length; ++i) {
        let obtainedMethod = classType.getInstanceMethodByName(methods[i])
        if (obtainedMethod != undefined) {
            throw new Error(`Given method '${methods[i]}' in tryGetPrivateInstanceMethodByName should be undefined`)
        }
    }
}

function testGetConstructors() {
    let ctorTestType = Class.of(new CtorsTest(1))
    let ctors: FixedArray<reflect.Constructor> = ctorTestType.getConstructors()
    arktest.assertEQ(ctors.length, CtorsTest.numOfCtors)

    for (let i = 0; i < ctors.length; ++i) {
        arktest.assertEQ(ctors[i].getName(), "constructor")
    }
}

function testGetStaticMethods() {
    let info = Class.of(new B())
    let expectedMethodsArr = ([ "keys", "keys", "assign", "hasOwn",
                                "hasOwn", "values", "values", "entries",
                                "entries", "fromEntries", "getOwnPropertyNames",
                                "getOwnPropertyNames", "stboo1", "stboo2",
                                "stfoo1", "stfoo2" ])
    let expectedMtdsSet: Set<string> = new Set<string>(expectedMethodsArr)

    let methods: FixedArray<reflect.StaticMethod> = info.getStaticMethods()
    arktest.assertEQ(methods.length, expectedMethodsArr.length)
    for (let i = 0; i < methods.length; ++i) {
        arktest.assertTrue(expectedMtdsSet.has(methods[i].getName()))
    }
}

function testGetStaticFields() {
    let info = Class.of(new B())
    let expectedFldSet: Set<string> = new Set<string>([ "stfooFld1", "stfooFld2", "stbooFld1", "stbooFld2"]);

    let fields: FixedArray<reflect.StaticField> = info.getStaticFields()
    arktest.assertEQ(fields.length, expectedFldSet.size)
    for (let i = 0; i < fields.length; ++i) {
        arktest.assertTrue(expectedFldSet.has(fields[i].getName()))
    }
}

function testGetInstanceFields() {
    let info = Class.of(new B())
    let expectedFldSet: Set<string> = new Set<string>([ "fooFld1", "fooFld2", "booFld1", "booFld2"]);

    let fields: FixedArray<reflect.InstanceField> = info.getInstanceFields()
    arktest.assertEQ(fields.length, expectedFldSet.size)
    for (let i = 0; i < fields.length; ++i) {
        arktest.assertTrue(expectedFldSet.has(fields[i].getName()))
    }
}

function testInstanceFieldEquals(): void {
    const aCls = Class.of(new A())

    const fld1 = aCls.getInstanceFieldByName("fooFld1")!
    const fld2 = aCls.getInstanceFieldByName("fooFld1")!

    arktest.assertFalse(fld1 == fld2)
    arktest.assertTrue(fld1.equals(fld2))

    const fld3 = aCls.getInstanceFieldByName("fooFld2")!

    arktest.assertFalse(fld1 == fld3)
    arktest.assertFalse(fld1.equals(fld3))
}

function classGetStaticFieldByName(cls: Class, name: string): reflect.StaticField | undefined {
    const fields = cls.getStaticFields()
    for (let i = 0; i < fields.length; i++) {
        const field = fields[i]
        if (field.getName() == name) {
            return field
        }

    }

    return undefined
}

function testStaticFieldEquals(): void {
    const aCls = Class.of(new A())

    const fld1 = classGetStaticFieldByName(aCls, "stfooFld1")!
    const fld2 = classGetStaticFieldByName(aCls, "stfooFld1")!

    arktest.assertFalse(fld1 == fld2)
    arktest.assertTrue(fld1.equals(fld2))

    const fld3 = classGetStaticFieldByName(aCls, "stfooFld2")!

    arktest.assertFalse(fld1 == fld3)
    arktest.assertFalse(fld1.equals(fld3))
}

function classGetStaticMethodByName(cls: Class, name: string): reflect.StaticMethod | undefined {
    const methods = cls.getStaticMethods()
    for (let i = 0; i < methods.length; i++) {
        const method = methods[i]
        if (method.getName() == name) {
            return method
        }
    }

    return undefined
}

function testStaticMethodEquals(): void {
    const aCls = Class.of(new A())

    const meth1 = classGetStaticMethodByName(aCls, 'stfoo1')!
    const meth2 = classGetStaticMethodByName(aCls, 'stfoo1')!

    arktest.assertFalse(meth1 == meth2)
    arktest.assertTrue(meth1.equals(meth2))

    const meth = classGetStaticMethodByName(aCls, 'stfoo2')!

    arktest.assertFalse(meth1 == meth)
    arktest.assertFalse(meth1.equals(meth))
}

function main(): int {
    let typeBaseObj = Class.of(new Object())

    testGetName(typeBaseObj, "std.core.Object")

    let typeA = Class.of(new A())

    testGetName(typeA, "ReflectTypeTest.A")

    let typeB = Class.of(new B())

    testGetName(typeB, "ReflectTypeTest.B")

    // Test getting base
    arktest.assertEQ(typeBaseObj.getSuper(), undefined)
    arktest.assertEQ(typeA.getSuper(), typeBaseObj)
    arktest.assertEQ(typeB.getSuper(), typeA)

    // Test methods getters
    let baseObjInstanceMethodsExpected =
        [ "toString", "toLocaleString", "$_hashCode", "hasOwnProperty", "hasOwnProperty" ]

    let baseObjStaticMethodsExpected =
        [ "keys", "keys", "assign", "hasOwn", "hasOwn", "values", "values", "entries",
          "entries", "fromEntries", "getOwnPropertyNames", "getOwnPropertyNames" ]

    testGetInstanceMethods(typeBaseObj, baseObjInstanceMethodsExpected)
    testGetInstanceMethods(typeA, baseObjInstanceMethodsExpected.concat(["someDefaultMethod", "foo1", "foo2"]))
    testGetInstanceMethods(typeB, baseObjInstanceMethodsExpected.concat(["someDefaultMethod", "foo1", "foo2", "boo1", "boo2"]))

    testGetInstanceMethodByName(typeBaseObj, baseObjInstanceMethodsExpected)
    testGetInstanceMethodByName(typeA, baseObjInstanceMethodsExpected.concat(["someDefaultMethod", "foo1", "foo2"]))
    testGetInstanceMethodByName(typeB, baseObjInstanceMethodsExpected.concat(["someDefaultMethod", "foo1", "foo2", "boo1", "boo2"]))

    tryGetPrivateInstanceMethodByName(typeA, [ "prfoo1", "prfoo2" ])

    testGetConstructors()
    testGetStaticMethods()
    testGetStaticFields()
    testGetInstanceFields()

    testInstanceFieldEquals()
    testStaticFieldEquals()
    testStaticMethodEquals()

    return 0
}
