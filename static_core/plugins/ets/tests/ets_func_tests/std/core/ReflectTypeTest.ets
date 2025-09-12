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

interface BaseIface {
    foo()
}

interface DerivedIface extends BaseIface {
    boo()
    defaultMethod(): int {
        return 0
    }
}

interface DerivedDerivedIface extends DerivedIface {
    foo()
    goo()
}

interface OverridedIface extends DerivedDerivedIface {
    foo()
    boo()
    goo()
    defaultMethod(): int {
        return 1
    }
}

interface ABase1 {
    abase1method()
}
interface ABase2 {
    abase2method()
}
interface BBase1 {
    bbase1method()
}
interface BBase2 {
    bbase2method()
}
interface AIface extends ABase1, ABase2 {
    amethod()
}
interface BIface extends BBase1, BBase2 {
    bmethod()
}

interface MultiExtendIface extends AIface, BIface {
}

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

type testUnion = A | B | Number | String

class C<T> {
    fld: testUnion = new A()
    fld1: String | FixedArray<A | FixedArray<Number | String >> = new String("help") // union
    fld2: A | undefined // not union
    fld3: int | long = 10 // union
    fld4: String | String = 'hello' // not union
    fld5: String | Number = 'world' // union
    fld6: 'lit1' | 'lit2' | 'lit3' = 'lit2' // not union
    fld7: T | undefined // not union
    fld8: T | null // not union
    fld9: FixedArray<String> | FixedArray<Number> = [ 1.0, 2.0, 3.0 ] // union
    fld10: A | null = null // union
    fld11: T | A // not union
    fld12: String | 'table' = 'desk' // not union
    fld13: Array<String> | Array <Int> = [ 'a' , 'b' ] // not union
    fld14: null | undefined // not union
    fld15: A | B = new A() // not union

    foo(a: testUnion): testUnion { return a }
    bar(a: FixedArray<int | string>): FixedArray<int | string> { return a }
}

function testGetName(type: Class, expected: string) {
    arktest.assertEQ(type.getName(), expected)
}

function testGetInstanceMethods(classType: Class, expectedMethodsArr: string[]) {
    let expectedMtdsSet: Set<string> = new Set<string>(expectedMethodsArr)

    let methods: FixedArray<reflect.InstanceMethod> = classType.getInstanceMethods()
    arktest.assertEQ(methods.length, expectedMethodsArr.length)

    for (let i = 0; i < methods.length; ++i) {
        arktest.assertTrue(expectedMtdsSet.has(methods[i].getName()), `failed on method: ${methods[i].getName()}`)
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

function testGetInterfaces(intfs: FixedArray<Class>, expectedIntfs: string[]) {
    arktest.assertEQ(intfs.length, expectedIntfs.length)

    let expectedIntfsSet = new Set<string>(expectedIntfs)

    for (let i = 0; i < intfs.length; ++i) {
        arktest.assertTrue(expectedIntfsSet.has(intfs[i].getName()), `failed on interface: ${intfs[i].getName()}`)
    }
}

function testOverridenInstanceMethodInInterface(intf: Class) {
    let methods = intf.getInstanceMethods()
    for (let i = 0; i < methods.length; ++i) {
        arktest.assertEQ(methods[i].getOwner().getName(), intf.getName())
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

function testIsUnion() {
    let clsC = new C<B>()
    let classType = Class.of(clsC)

    let fldInfo1 = classType.getInstanceFieldByName('fld1')
    let uniInfo1 = fldInfo1!.getType()
    let constituentTypes1 = uniInfo1!.getUnionConstituentTypes()
    arktest.assertTrue(uniInfo1!.isUnion())
    arktest.assertNE(constituentTypes1, undefined)

    let fldInfo2 = classType.getInstanceFieldByName('fld2')
    let uniInfo2 = fldInfo2!.getType()
    let constituentTypes2 = uniInfo2!.getUnionConstituentTypes()
    arktest.assertFalse(uniInfo2!.isUnion())
    arktest.assertEQ(constituentTypes2, undefined)

    let fldInfo3 = classType.getInstanceFieldByName('fld3')
    let uniInfo3 = fldInfo3!.getType()
    let constituentTypes3 = uniInfo3!.getUnionConstituentTypes()
    arktest.assertTrue(uniInfo3!.isUnion())
    arktest.assertNE(constituentTypes3, undefined)

    let fldInfo4 = classType.getInstanceFieldByName('fld4')
    let uniInfo4 = fldInfo4!.getType()
    let constituentTypes4 = uniInfo4!.getUnionConstituentTypes()
    arktest.assertFalse(uniInfo4!.isUnion())
    arktest.assertEQ(constituentTypes4, undefined)

    let fldInfo5 = classType.getInstanceFieldByName('fld5')
    let uniInfo5 = fldInfo5!.getType()
    let constituentTypes5 = uniInfo5!.getUnionConstituentTypes()
    arktest.assertTrue(uniInfo5!.isUnion())
    arktest.assertNE(constituentTypes5, undefined)

    let fldInfo6 = classType.getInstanceFieldByName('fld6')
    let uniInfo6 = fldInfo6!.getType()
    let constituentTypes6 = uniInfo6!.getUnionConstituentTypes()
    arktest.assertFalse(uniInfo6!.isUnion())
    arktest.assertEQ(constituentTypes6, undefined)

    let fldInfo7 = classType.getInstanceFieldByName('fld7')
    let uniInfo7 = fldInfo7!.getType()
    let constituentTypes7 = uniInfo7!.getUnionConstituentTypes()
    arktest.assertFalse(uniInfo7!.isUnion())
    arktest.assertEQ(constituentTypes7, undefined)

    let fldInfo8 = classType.getInstanceFieldByName('fld8')
    let uniInfo8 = fldInfo8!.getType()
    let constituentTypes8 = uniInfo8!.getUnionConstituentTypes()
    arktest.assertFalse(uniInfo8!.isUnion())
    arktest.assertEQ(constituentTypes8, undefined)

    let fldInfo9 = classType.getInstanceFieldByName('fld9')
    let uniInfo9 = fldInfo9!.getType()
    let constituentTypes9 = uniInfo9!.getUnionConstituentTypes()
    arktest.assertTrue(uniInfo9!.isUnion())
    arktest.assertNE(constituentTypes9, undefined)

    let fldInfo10 = classType.getInstanceFieldByName('fld10')
    let uniInfo10 = fldInfo10!.getType()
    let constituentTypes10 = uniInfo10!.getUnionConstituentTypes()
    arktest.assertTrue(uniInfo10!.isUnion())
    arktest.assertNE(constituentTypes10, undefined)
    arktest.assertEQ(constituentTypes10!.length, 2)
    arktest.assertEQ(constituentTypes10![0].getName(), 'ReflectTypeTest.A')
    arktest.assertEQ(constituentTypes10![1].getName(), 'std.core.Null')

    let fldInfo11 = classType.getInstanceFieldByName('fld11')
    let uniInfo11 = fldInfo11!.getType()
    let constituentTypes11 = uniInfo11!.getUnionConstituentTypes()
    arktest.assertFalse(uniInfo11!.isUnion())
    arktest.assertEQ(constituentTypes11, undefined)

    let fldInfo12 = classType.getInstanceFieldByName('fld12')
    let uniInfo12 = fldInfo12!.getType()
    let constituentTypes12 = uniInfo12!.getUnionConstituentTypes()
    arktest.assertFalse(uniInfo12!.isUnion())
    arktest.assertEQ(constituentTypes12, undefined)

    let fldInfo13 = classType.getInstanceFieldByName('fld13')
    let uniInfo13 = fldInfo13!.getType()
    let constituentTypes13 = uniInfo13!.getUnionConstituentTypes()
    arktest.assertFalse(uniInfo13!.isUnion())
    arktest.assertEQ(constituentTypes13, undefined)

    let fldInfo14 = classType.getInstanceFieldByName('fld14')
    let uniInfo14 = fldInfo14!.getType()
    let constituentTypes14 = uniInfo14!.getUnionConstituentTypes()
    arktest.assertFalse(uniInfo14!.isUnion())
    arktest.assertEQ(constituentTypes14, undefined)

    let fldInfo15 = classType.getInstanceFieldByName('fld15')
    let uniInfo15 = fldInfo15!.getType()
    let constituentTypes15 = uniInfo15!.getUnionConstituentTypes()
    arktest.assertFalse(uniInfo15!.isUnion())
    arktest.assertEQ(constituentTypes15, undefined)
}

function testUnionField() {
    let clsC = new C<String>()
    let classType = Class.of(clsC)
    let fldInfo = classType.getInstanceFieldByName('fld')
    let uniInfo = fldInfo!.getType()
    arktest.assertTrue(uniInfo!.isUnion())
    let uniTypes = uniInfo!.getUnionConstituentTypes()
    let expectedTypeSet: Set<string> = new Set<string>([ 'ReflectTypeTest.A', 'std.core.Double', 'std.core.String' ]);

    arktest.assertEQ(uniTypes!.length, expectedTypeSet.size)
    for (let i = 0; i < uniTypes!.length; ++i) {
        arktest.assertTrue(expectedTypeSet.has(uniTypes![i].getName()))
    }
}

function testUnionParameter() {
    let clsC = new C<String>()
    let classType = Class.of(clsC)
    let methodInfo1 = classType.getInstanceMethodByName('foo')
    let paramType1 = methodInfo1!.getParameterType(0)
    arktest.assertTrue(paramType1!.isUnion())
    let uniTypes1 = paramType1!.getUnionConstituentTypes()
    let expectedTypeSet1: Set<string> = new Set<string>([ 'ReflectTypeTest.A', 'std.core.Double', 'std.core.String' ]);

    arktest.assertEQ(uniTypes1!.length, expectedTypeSet1.size)
    for (let i = 0; i < uniTypes1!.length; ++i) {
        arktest.assertTrue(expectedTypeSet1.has(uniTypes1![i].getName()))
    }

    let methodInfo2 = classType.getInstanceMethodByName('bar')
    let paramType2 = methodInfo2!.getParameterType(0)
    arktest.assertTrue(paramType2!.isFixedArray())
    let componentType = paramType2!.getFixedArrayComponentType()
    arktest.assertTrue(componentType!.isUnion())
    let uniTypes2 = componentType!.getUnionConstituentTypes()
    let expectedTypeSet2: Set<string> = new Set<string>([ 'std.core.Int', 'std.core.String' ]);

    arktest.assertEQ(uniTypes2!.length, expectedTypeSet2.size)
    for (let i = 0; i < uniTypes2!.length; ++i) {
        arktest.assertTrue(expectedTypeSet2.has(uniTypes2![i].getName()))
    }
}

function testUnionRet() {
    let clsC = new C<String>()
    let classType = Class.of(clsC)
    let methodInfo = classType.getInstanceMethodByName('foo')
    let retClass = methodInfo!.getReturnType()
    arktest.assertTrue(retClass!.isUnion())
    let uniTypes = retClass.getUnionConstituentTypes()
    let expectedTypeSet: Set<string> = new Set<string>([ 'ReflectTypeTest.A', 'std.core.Double', 'std.core.String' ]);

    arktest.assertEQ(uniTypes!.length, expectedTypeSet.size)
    for (let i = 0; i < uniTypes!.length; ++i) {
        arktest.assertTrue(expectedTypeSet.has(uniTypes![i].getName()))
    }
}

function main(): int {
    let typeBaseObj = Class.of(new Object())

    testGetName(typeBaseObj, "std.core.Object")

    let typeA = Class.of(new A())

    testGetName(typeA, "ReflectTypeTest.A")

    let typeB = Class.of(new B())

    testGetName(typeB, "ReflectTypeTest.B")

    // Test getSuper()
    arktest.assertEQ(typeBaseObj.getSuper(), undefined)
    arktest.assertEQ(typeA.getSuper(), typeBaseObj)
    arktest.assertEQ(typeB.getSuper(), typeA)

    arktest.assertEQ((Type.from<BaseIface>() as InterfaceType).getClass().getSuper(), undefined)
    arktest.assertEQ((Type.from<DerivedIface>() as InterfaceType).getClass().getSuper(), undefined)
    arktest.assertEQ((Type.from<DerivedDerivedIface>() as InterfaceType).getClass().getSuper(), undefined)

    // GetInterfaces()
    let emptyClass: Class[] = []
    arktest.assertEQ((Type.from<BaseIface>() as InterfaceType).getClass().getInterfaces().length, 0)
    testGetInterfaces((Type.from<DerivedIface>() as InterfaceType).getClass().getInterfaces(), ["ReflectTypeTest.BaseIface"])
    testGetInterfaces((Type.from<DerivedDerivedIface>() as InterfaceType).getClass().getInterfaces(), ["ReflectTypeTest.DerivedIface"])

    // Test methods getters
    let baseObjInstanceMethodsExpected =
        [ "toString", "toLocaleString", "$_hashCode", "hasOwnProperty", "hasOwnProperty" ]

    let baseObjStaticMethodsExpected =
        [ "keys", "keys", "assign", "hasOwn", "hasOwn", "values", "values", "entries",
          "entries", "fromEntries", "getOwnPropertyNames", "getOwnPropertyNames" ]

    testGetInstanceMethods(typeBaseObj, baseObjInstanceMethodsExpected)
    testGetInstanceMethods(typeA, baseObjInstanceMethodsExpected.concat(["someDefaultMethod", "foo1", "foo2"]))
    testGetInstanceMethods(typeB, baseObjInstanceMethodsExpected.concat(["someDefaultMethod", "foo1", "foo2", "boo1", "boo2"]))

    // Test instanceMethods for interfaces
    testGetInstanceMethods((Type.from<BaseIface>() as InterfaceType).getClass(), ["foo"])
    testGetInstanceMethods((Type.from<DerivedIface>() as InterfaceType).getClass(), ["foo", "boo", "defaultMethod"])
    testGetInstanceMethods((Type.from<DerivedDerivedIface>() as InterfaceType).getClass(), ["foo", "goo", "boo", "defaultMethod"])

    testGetInstanceMethods((Type.from<AIface>() as InterfaceType).getClass(), ["amethod", "abase1method", "abase2method"])
    testGetInstanceMethods((Type.from<BIface>() as InterfaceType).getClass(), ["bmethod", "bbase1method", "bbase2method"])
    testGetInstanceMethods((Type.from<MultiExtendIface>() as InterfaceType).getClass(), ["amethod", "bmethod",
        "abase1method", "abase2method", "bbase1method", "bbase2method"])

    testOverridenInstanceMethodInInterface((Type.from<OverridedIface>() as InterfaceType).getClass())

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

    testIsUnion()
    testUnionField()
    testUnionParameter()
    testUnionRet()

    return 0
}
