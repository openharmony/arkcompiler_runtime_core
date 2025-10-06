/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

interface Serializable {
    serialize(): String
}

class Point implements Serializable {
    protected xP: double
    x: double
    y: double

    override serialize(): String {
        return "{" + this.x + "," + this.y + "}"
    }

    add(oth: Point): Point {
        let res = new Point()
        res.x = this.x + oth.x
        res.y = this.y + oth.y
        return res
    }
}

class Point3D extends Point {
    z: double
    y: double = 3.0
    private stateP: int
    state: int
    static ZERO: double = 0.0
    readonly r: int = 1
    ability: String | Number = new Number(1)
}

function testField(): void {
    let typePoint3D = Class.of(new Point3D())

    arktest.assertEQ(typePoint3D.getInstanceFieldByName("xP"), undefined) // protected
    let xf = typePoint3D.getInstanceFieldByName("x")!
    let yf = typePoint3D.getInstanceFieldByName("y")!
    arktest.assertEQ(typePoint3D.getInstanceFieldByName("stateP"), undefined) // private
    let sf = typePoint3D.getInstanceFieldByName("state")!
    let rf = typePoint3D.getInstanceFieldByName("r")!
    let abilityf = typePoint3D.getInstanceFieldByName("ability")!

    let isInherited = (f: reflect.InstanceField, type: Class): boolean => {
        return f.getOwner().getName() != type.getName()
    }
    let isOverrided = (f: reflect.InstanceField, type: Class): boolean => {
        if (f.getOwner().getName() != type.getName()) {
            return false
        }
        let baseType = type.getSuper()!
        return baseType.getInstanceFieldByName(f.getName()) != undefined
    }

    arktest.assertEQ(xf.getName(), "x")
    arktest.assertEQ(xf.getOwner(), Class.of(new Point()))
    arktest.assertEQ(xf.getType(), Class.PRIMITIVE_DOUBLE)
    arktest.assertFalse(xf.isReadonly())
    arktest.assertTrue(xf.isPublic())
    arktest.assertFalse(xf.isProtected())
    arktest.assertFalse(xf.isPrivate())
    arktest.assertTrue(isInherited(xf, typePoint3D))
    arktest.assertFalse(isOverrided(xf, typePoint3D))
    arktest.assertEQ(yf.getName(), "y")
    arktest.assertEQ(yf.getOwner(), typePoint3D)
    arktest.assertEQ(yf.getType(), Class.PRIMITIVE_DOUBLE)
    arktest.assertFalse(yf.isReadonly())
    arktest.assertTrue(yf.isPublic())
    arktest.assertFalse(yf.isProtected())
    arktest.assertFalse(yf.isPrivate())
    arktest.assertFalse(isInherited(yf, typePoint3D))
    arktest.assertTrue(isOverrided(yf, typePoint3D))
    arktest.assertEQ(sf.getName(), "state")
    arktest.assertEQ(sf.getOwner(), typePoint3D)
    arktest.assertEQ(sf.getType(), Class.PRIMITIVE_INT)
    arktest.assertFalse(sf.isReadonly())
    arktest.assertTrue(sf.isPublic())
    arktest.assertFalse(sf.isProtected())
    arktest.assertFalse(sf.isPrivate())
    arktest.assertFalse(isInherited(sf, typePoint3D))
    arktest.assertFalse(isOverrided(sf, typePoint3D))
    arktest.assertEQ(rf.getName(), "r")
    arktest.assertEQ(rf.getOwner(), typePoint3D)
    arktest.assertEQ(rf.getType(), Class.PRIMITIVE_INT)
    arktest.assertTrue(rf.isReadonly())
    arktest.assertTrue(rf.isPublic())
    arktest.assertFalse(rf.isProtected())
    arktest.assertFalse(rf.isPrivate())
    arktest.assertFalse(isInherited(rf, typePoint3D))
    arktest.assertFalse(isOverrided(rf, typePoint3D))
    arktest.assertEQ(abilityf.getName(), "ability")
    arktest.assertEQ(abilityf.getOwner(), typePoint3D)
}

class UClassA {}
class UClassB {}

enum TestFieldEnum {
    ONE,
    TWO,
    THREE
}

enum WrongEnum {
    ONE,
    TWO,
    THREE
}

class Parent {
    private static PRIV: boolean = true
    protected static PROT: string = 'prot'

    static ID: number = 1.0
    static STR: string = ''
    static UN: UClassA | UClassB | number = 42
    static readonly RDONLYPRIM: int = 8
    static readonly RDONLYSTR: string = ''
    static TPLE: [int, string] = [42, "str"]
    static ENM = TestFieldEnum.ONE

    static get PROP(): number {
        return 1
    }

    get prop(): number {
        return 0
    }

    id: number = 0
    str: string = ''
    readonly rdonlyPrim: int = 8
    readonly rdonlyStr: string = ''
    un: UClassA | UClassB | number = 42
    tple: [int, string] = [42, "str"]
    enm: TestFieldEnum = TestFieldEnum.ONE
    private priv = true
    protected prot = true
}

class Child extends Parent {
    static ID: number = 2.0
    static EOL: char = c'\n'
    public EOL: string = '\n'

    id: number = 1
    str: string = ''
}

function testInstanceGetSetPrimitive(): void {
    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentIdFld = parentCls.getInstanceFieldByName('id')!
    const localStr = 'I\'m a string'
    const localNum = 3.14

    arktest.assertNE(localNum, parentInst.id)
    arktest.assertNE(localNum, parentIdFld.getValue(parentInst))
    parentIdFld.setValue(parentInst, localNum)
    arktest.assertEQ(localNum, parentInst.id)
    arktest.assertEQ(localNum, parentIdFld.getValue(parentInst))

    const childInst = new Child()
    const childCls = Class.of(childInst)
    const childIdFld = childCls.getInstanceFieldByName('id')!

    arktest.assertNE(localNum, childInst.id)
    arktest.assertNE(localNum, childIdFld.getValue(childInst))
    childIdFld.setValue(childInst, localNum)
    arktest.assertEQ(localNum, childInst.id)
    arktest.assertEQ(localNum, childIdFld.getValue(childInst))
}

function testInstanceGetSetReference(): void {
    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentStrFld = parentCls.getInstanceFieldByName('str')!
    const parentIdFld = parentCls.getInstanceFieldByName('id')!
    const localStr = 'I\'m a string'
    const localNum = parentInst.id

    arktest.assertNE(localStr, parentInst.str)
    arktest.assertNE(localStr, parentStrFld.getValue(parentInst))
    parentStrFld.setValue(parentInst, localStr)
    arktest.assertEQ(localStr, parentInst.str)
    arktest.assertEQ(localNum, parentIdFld.getValue(parentInst))

    parentStrFld.setValue(parentInst, undefined)
    arktest.assertEQ(undefined, parentInst.str)
    arktest.assertEQ(undefined, parentStrFld.getValue(parentInst))
    parentInst.str = localStr

    const childInst = new Child()
    const childCls = Class.of(childInst)
    const childStrFld = childCls.getInstanceFieldByName('str')!

    arktest.assertNE(localStr, childInst.str)
    arktest.assertNE(localStr, childStrFld.getValue(childInst))
    childStrFld.setValue(childInst, localStr)
    arktest.assertEQ(localStr, childInst.str)
    arktest.assertEQ(localNum, parentIdFld.getValue(childInst))

    childStrFld.setValue(childInst, undefined)
    arktest.assertEQ(undefined, childInst.str)
    arktest.assertEQ(undefined, childStrFld.getValue(childInst))
    childInst.str = localStr
}

function testInstanceGetSetUnion(): void {
    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentUnFld = parentCls.getInstanceFieldByName('un')!
    const localUA = new UClassA()
    const localUB = new UClassB()
    const localNum = 3.14

    arktest.assertNE(localNum, parentInst.un)
    arktest.assertNE(localUA, parentInst.un)
    arktest.assertNE(localUB, parentInst.un)
    arktest.assertNE(localNum, parentUnFld.getValue(parentInst))
    parentUnFld.setValue(parentInst, localNum)
    arktest.assertEQ(localNum, parentInst.un)
    arktest.assertEQ(localNum, parentUnFld.getValue(parentInst))

    arktest.assertNE(localUA, parentInst.un)
    arktest.assertNE(localUB, parentInst.un)
    arktest.assertNE(localUA, parentUnFld.getValue(parentInst))
    parentUnFld.setValue(parentInst, localUA)
    arktest.assertEQ(localUA, parentInst.un)
    arktest.assertEQ(localUA, parentUnFld.getValue(parentInst))

    arktest.assertNE(localUB, parentInst.un)
    arktest.assertNE(localNum, parentInst.un)
    arktest.assertNE(localUB, parentUnFld.getValue(parentInst))
    parentUnFld.setValue(parentInst, localUB)
    arktest.assertEQ(localUB, parentInst.un)
    arktest.assertEQ(localUB, parentUnFld.getValue(parentInst))
}

function testInstanceGetSetTuple(): void {
    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentTpleFld = parentCls.getInstanceFieldByName('tple')!
    const localTuple: [int, string] = [0, "local"]

    arktest.assertNE(localTuple, parentInst.tple)
    arktest.assertNE(localTuple, parentTpleFld.getValue(parentInst))
    parentTpleFld.setValue(parentInst, localTuple)
    arktest.assertEQ(localTuple, parentInst.tple)
    arktest.assertEQ(localTuple, parentTpleFld.getValue(parentInst))

    const undefTuple: [int | undefined, string | undefined] = [undefined, undefined]
    parentTpleFld.setValue(parentInst, undefTuple)
    arktest.assertEQ(undefTuple, parentInst.tple)
    arktest.assertEQ(undefTuple, parentTpleFld.getValue(parentInst))

    const childInst = new Child()
    const childCls = Class.of(childInst)
    const childTpleFld = childCls.getInstanceFieldByName('tple')!

    arktest.assertNE(localTuple, childInst.tple)
    arktest.assertNE(localTuple, childTpleFld.getValue(childInst))
    childTpleFld.setValue(childInst, localTuple)
    arktest.assertEQ(localTuple, childInst.tple)
    arktest.assertEQ(localTuple, childTpleFld.getValue(childInst))

    childTpleFld.setValue(childInst, undefTuple)
    arktest.assertEQ(undefTuple, childInst.tple)
    arktest.assertEQ(undefTuple, childTpleFld.getValue(childInst))
}

function testInstanceGetSetEnum(): void {
    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentEnmFld = parentCls.getInstanceFieldByName('enm')!
    const localEnum = TestFieldEnum.TWO

    arktest.assertNE(localEnum, parentInst.enm)
    arktest.assertNE(localEnum, parentEnmFld.getValue(parentInst))
    parentEnmFld.setValue(parentInst, localEnum)
    arktest.assertEQ(localEnum, parentInst.enm)
    arktest.assertEQ(localEnum, parentEnmFld.getValue(parentInst))
}

function testInstanceGetSetPrimitiveNegative(): void {
    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentStrFld = parentCls.getInstanceFieldByName('str')!
    const parentIdFld = parentCls.getInstanceFieldByName('id')!
    const parentRdonlyPrimFld = parentCls.getInstanceFieldByName('rdonlyPrim')!
    const localStr = 'I\'m a string'
    const localNumParent = parentInst.id

    arktest.expectError(() => {
        parentRdonlyPrimFld.setValue(parentInst, 42)
    }, new TypeError("setValue to a readonly field"))
    arktest.assertEQ(8 , parentInst.rdonlyPrim)
    arktest.assertEQ(parentRdonlyPrimFld.getValue(parentInst) , parentInst.rdonlyPrim)

    arktest.expectError(() => {
        parentIdFld.setValue(parentInst, undefined)
    }, new NullPointerError("undefined argument is not allowed for primitive reciever"))
    arktest.assertNE(undefined, parentIdFld.getValue(parentInst))
    arktest.assertNE(undefined, parentInst.id)
    arktest.assertEQ(localNumParent, parentInst.id)
    arktest.assertEQ(localNumParent, parentIdFld.getValue(parentInst))

    arktest.expectError(() => {
        parentIdFld.setValue(parentInst, localStr)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertNE(localStr, parentIdFld.getValue(parentInst))
    arktest.assertNE(localStr, parentInst.id)
    arktest.assertEQ(localNumParent, parentInst.id)
    arktest.assertEQ(localNumParent, parentIdFld.getValue(parentInst))

    const childInst = new Child()
    const childCls = Class.of(childInst)
    const childIdFld = childCls.getInstanceFieldByName('id')!
    const localNumChild = childInst.id

    arktest.expectError(() => {
        childIdFld.setValue(childInst, undefined)
    }, new NullPointerError("undefined argument is not allowed for primitive reciever"))
    arktest.assertNE(undefined, childIdFld.getValue(childInst))
    arktest.assertNE(undefined, childInst.id)
    arktest.assertEQ(localNumChild, childInst.id)
    arktest.assertEQ(localNumChild, childIdFld.getValue(childInst))

    arktest.expectError(() => {
        childIdFld.setValue(childInst, localStr)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertNE(localStr, childInst.id)
    arktest.assertEQ(localNumChild, childInst.id)
}

function testInstanceGetSetReferenceNegative(): void {
    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentStrFld = parentCls.getInstanceFieldByName('str')!
    const parentRdonlyStrFld = parentCls.getInstanceFieldByName('rdonlyStr')!
    const localStrParent = parentInst.str
    const localNum = 3.14

    arktest.expectError(() => {
        parentRdonlyStrFld.setValue(parentInst, 'str')
    }, new TypeError("setValue to a readonly field"))
    arktest.assertEQ('' , parentInst.rdonlyStr)
    arktest.assertEQ(parentRdonlyStrFld.getValue(parentInst) , parentInst.rdonlyStr)

    arktest.expectError(() => {
        parentStrFld.setValue(parentInst, localNum)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertNE(localNum, parentInst.str)
    arktest.assertNE(localNum, parentStrFld.getValue(parentInst))
    arktest.assertEQ(localStrParent, parentInst.str)
    arktest.assertEQ(localStrParent, parentStrFld.getValue(parentInst))

    const childInst = new Child()
    const childCls = Class.of(childInst)
    const childStrFld = childCls.getInstanceFieldByName('str')!
    const localStrChild = childInst.str

    arktest.expectError(() => {
        childStrFld.setValue(childInst, localNum)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertNE(localNum, childInst.str)
    arktest.assertEQ(localStrChild, childInst.str)
}

function testInstanceGetSetUnionNegative(): void {
    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentUnFld = parentCls.getInstanceFieldByName('un')!
    const localStr = 'I\'m a string'

    arktest.expectError(() => {
        parentUnFld.setValue(parentInst, localStr)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertNE(localStr, parentInst.un)
    arktest.assertNE(localStr, parentUnFld.getValue(parentInst))
    arktest.assertEQ(42, parentInst.un)
    arktest.assertEQ(42, parentUnFld.getValue(parentInst))
}

function testInstanceGetSetTupleNegative(): void {
    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentTpleFld = parentCls.getInstanceFieldByName('tple')!
    const localTuple: [number, Object, string] = [3.14, new Object(), '']
    const originalTuple = parentInst.tple

    arktest.expectError(() => {
        parentTpleFld.setValue(parentInst, localTuple)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertEQ(originalTuple, parentInst.tple)
    arktest.assertEQ(originalTuple, parentTpleFld.getValue(parentInst))
}

function testInstanceGetSetEnumNegative(): void {
    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentEnmFld = parentCls.getInstanceFieldByName('enm')!
    const localEnum = WrongEnum.ONE
    const originalEnum = parentInst.enm

    arktest.expectError(() => {
        parentEnmFld.setValue(parentInst, localEnum)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertEQ(originalEnum, parentInst.enm)
    arktest.assertEQ(originalEnum, parentEnmFld.getValue(parentInst))
}

function testStaticGetSetPrimitive(): void {
    const parentCls = Class.from<Parent>()
    const parentIdFld = parentCls.getStaticFieldByName('ID')!
    const localNum = 3.14

    arktest.assertNE(localNum, Parent.ID)
    arktest.assertNE(localNum, parentIdFld.getValue())
    parentIdFld.setValue(localNum)
    arktest.assertEQ(localNum, Parent.ID)
    arktest.assertEQ(localNum, parentIdFld.getValue())

    const childCls = Class.from<Child>()
    const childIdFld = childCls.getStaticFieldByName('ID')!

    arktest.assertNE(localNum, Child.ID)
    arktest.assertNE(localNum, childIdFld.getValue())
    childIdFld.setValue(localNum)
    arktest.assertEQ(localNum, Child.ID)
    arktest.assertEQ(localNum, childIdFld.getValue())
}

function testStaticGetSetReference(): void {
    const parentCls = Class.from<Parent>()
    const parentStrFld = parentCls.getStaticFieldByName('STR')!
    const localStr = 'I\'m a string'

    arktest.assertNE(localStr, Parent.STR)
    arktest.assertNE(localStr, parentStrFld.getValue())
    parentStrFld.setValue(localStr)
    arktest.assertEQ(localStr, Parent.STR)
    arktest.assertEQ(localStr, parentStrFld.getValue())
}

function testStaticGetSetUnion(): void {
    const parentCls = Class.from<Parent>()
    const parentUnFld = parentCls.getStaticFieldByName('UN')!
    const localUA = new UClassA()
    const localUB = new UClassB()
    const localNum = 3.14
    Parent.UN = 42

    arktest.assertNE(localNum, Parent.UN)
    arktest.assertNE(localUA, Parent.UN)
    arktest.assertNE(localUB, Parent.UN)
    arktest.assertNE(localNum, parentUnFld.getValue())
    parentUnFld.setValue(localNum)
    arktest.assertEQ(localNum, Parent.UN)
    arktest.assertEQ(localNum, parentUnFld.getValue())

    arktest.assertNE(localUA, Parent.UN)
    arktest.assertNE(localUB, Parent.UN)
    arktest.assertNE(localUA, parentUnFld.getValue())
    parentUnFld.setValue(localUA)
    arktest.assertEQ(localUA, Parent.UN)
    arktest.assertEQ(localUA, parentUnFld.getValue())

    arktest.assertNE(localUB, Parent.UN)
    arktest.assertNE(localNum, Parent.UN)
    arktest.assertNE(localUB, parentUnFld.getValue())
    parentUnFld.setValue(localUB)
    arktest.assertEQ(localUB, Parent.UN)
    arktest.assertEQ(localUB, parentUnFld.getValue())
}

function testStaticGetSetTuple(): void {
    const parentCls = Class.from<Parent>()
    const parentTpleFld = parentCls.getStaticFieldByName('TPLE')!
    const localTuple: [int, string] = [0, "local"]
    const originalTuple = Parent.TPLE

    arktest.assertNE(localTuple, Parent.TPLE)
    arktest.assertNE(localTuple, parentTpleFld.getValue())
    parentTpleFld.setValue(localTuple)
    arktest.assertEQ(localTuple, Parent.TPLE)
    arktest.assertEQ(localTuple, parentTpleFld.getValue())

    Parent.TPLE = originalTuple
}

function testStaticGetSetEnum(): void {
    const parentCls = Class.from<Parent>()
    const parentEnmFld = parentCls.getStaticFieldByName('ENM')!
    const localEnum = TestFieldEnum.TWO

    arktest.assertNE(localEnum, Parent.ENM)
    arktest.assertNE(localEnum, parentEnmFld.getValue())
    parentEnmFld.setValue(localEnum)
    arktest.assertEQ(localEnum, Parent.ENM)
    arktest.assertEQ(localEnum, parentEnmFld.getValue())
}

function testStaticGetSetPrimitiveNegative(): void {
    const parentCls = Class.from<Parent>()
    const parentIdFld = parentCls.getStaticFieldByName('ID')!
    const parentRdonlyPrimFld = parentCls.getStaticFieldByName('RDONLYPRIM')!
    const localNum = 3.14
    const localStr = 'I\'m a string'

    arktest.expectError(() => {
        parentRdonlyPrimFld.setValue(42)
    }, new TypeError("setValue to a readonly field"))
    arktest.assertEQ(8 , Parent.RDONLYPRIM)
    arktest.assertEQ(parentRdonlyPrimFld.getValue() , Parent.RDONLYPRIM)

    arktest.expectError(() => {
        parentIdFld.setValue(localStr)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertNE(localStr, Parent.ID)
    arktest.assertNE(localStr, parentIdFld.getValue())
    arktest.assertEQ(localNum, Parent.ID)
    arktest.assertEQ(localNum, parentIdFld.getValue())
}

function testStaticGetSetReferenceNegative(): void {
    const parentCls = Class.from<Parent>()
    const parentStrFld = parentCls.getStaticFieldByName('STR')!
    const localNum = 3.14
    const localStr = 'I\'m a string'

    arktest.expectError(() => {
        parentStrFld.setValue(localNum)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertNE(localNum, Parent.STR)
    arktest.assertNE(localNum, parentStrFld.getValue())
    arktest.assertEQ(localStr, Parent.STR)
    arktest.assertEQ(localStr, parentStrFld.getValue())
}

function testStaticGetSetUnionNegative(): void {
    const parentCls = Class.from<Parent>()
    const parentUnFld = parentCls.getStaticFieldByName('UN')!
    const localStr = 'I\'m a string'
    Parent.UN = 42

    arktest.expectError(() => {
        parentUnFld.setValue(localStr)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertNE(localStr, Parent.UN)
    arktest.assertNE(localStr, parentUnFld.getValue())
    arktest.assertEQ(42, Parent.UN)
    arktest.assertEQ(42, parentUnFld.getValue())
}

function testStaticGetSetTupleNegative(): void {
    const parentCls = Class.from<Parent>()
    const parentTpleFld = parentCls.getStaticFieldByName('TPLE')!
    const localTuple: [number, Object, string] = [3.14, new Object(), '']
    const originalTuple = Parent.TPLE

    arktest.expectError(() => {
        parentTpleFld.setValue(localTuple)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertEQ(originalTuple, Parent.TPLE)
    arktest.assertEQ(originalTuple, parentTpleFld.getValue())
}

function testStaticGetSetEnumNegative(): void {
    const parentCls = Class.from<Parent>()
    const parentEnmFld = parentCls.getStaticFieldByName('ENM')!
    const localEnum = WrongEnum.TWO
    const originalEnum = Parent.ENM

    arktest.expectError(() => {
        parentEnmFld.setValue(localEnum)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertEQ(originalEnum, Parent.ENM)
    arktest.assertEQ(originalEnum, parentEnmFld.getValue())
}

function testInstanceField(): void {
    testInstanceGetSetPrimitive()
    testInstanceGetSetReference()
    testInstanceGetSetUnion()
    testInstanceGetSetTuple()
    testInstanceGetSetEnum()
    testInstanceGetSetPrimitiveNegative()
    testInstanceGetSetReferenceNegative()
    testInstanceGetSetUnionNegative()
    testInstanceGetSetTupleNegative()
    testInstanceGetSetEnumNegative()

    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentIdFld = parentCls.getInstanceFieldByName('id')!
    const parentStrFld = parentCls.getInstanceFieldByName('str')!

    arktest.assertEQ(parentIdFld.getType(), Class.PRIMITIVE_NUMBER)
    arktest.assertEQ(parentIdFld.getOwner(), parentCls)
    arktest.assertEQ(parentStrFld.getType(), Class.of(new String()))
    arktest.assertEQ(parentStrFld.getOwner(), parentCls)

    const childInst = new Child()
    const childCls = Class.of(childInst)
    const childIdFld = childCls.getInstanceFieldByName('id')!
    const childStrFld = childCls.getInstanceFieldByName('str')!

    arktest.assertEQ(childIdFld.getType(), Class.PRIMITIVE_NUMBER)
    arktest.assertEQ(childIdFld.getOwner(), childCls)
    arktest.assertEQ(childStrFld.getType(), Class.of(new String()))
    arktest.assertEQ(childStrFld.getOwner(), childCls)

    arktest.assertEQ(parentCls.getInstanceFieldByName('prop'), undefined)
    arktest.assertEQ(parentCls.getInstanceFieldByName('priv'), undefined)
    arktest.assertEQ(parentCls.getInstanceFieldByName('prot'), undefined)
    arktest.assertEQ(childCls.getInstanceFieldByName('prot'), undefined)

    const parentFlds = parentCls.getInstanceFields()
    const parentFldNames = new Set<string>(Array.from<reflect.InstanceField, string>(parentFlds, (f) => f.getName()))
    arktest.assertFalse(parentFldNames.has('priv'))
    arktest.assertFalse(parentFldNames.has('prot'))
    arktest.assertTrue(parentFldNames.has('id'))

    const childFlds = childCls.getInstanceFields()
    const childFldNames = new Set<string>(Array.from<reflect.InstanceField, string>(childFlds, (f) => f.getName()))
    arktest.assertFalse(childFldNames.has('prot'))
    arktest.assertTrue(parentFldNames.has('id'))

    const bools: FixedArray<boolean> = [true]
    const boolsCls = Class.of(bools)
    arktest.assertEQ(boolsCls.getInstanceFieldByName('length'), undefined)
}

function testStaticField(): void {
    testStaticGetSetPrimitive()
    testStaticGetSetReference()
    testStaticGetSetUnion()
    testStaticGetSetTuple()
    testStaticGetSetEnum()
    testStaticGetSetPrimitiveNegative()
    testStaticGetSetReferenceNegative()
    testStaticGetSetUnionNegative()
    testStaticGetSetTupleNegative()
    testStaticGetSetEnumNegative()

    const parentCls = Class.of(new Parent)
    const parentIdFld = parentCls.getStaticFieldByName('ID')!
    const parentStrFld = parentCls.getStaticFieldByName('STR')!

    arktest.assertEQ(parentIdFld.getType(), Class.PRIMITIVE_NUMBER)
    arktest.assertEQ(parentIdFld.getOwner(), parentCls)
    arktest.assertEQ(parentStrFld.getType(), Class.of(new String()))
    arktest.assertEQ(parentStrFld.getOwner(), parentCls)
    arktest.assertEQ(parentCls.getStaticFieldByName('EOL'), undefined)

    const childCls = Class.of(new Child)
    const childIdFld = childCls.getStaticFieldByName('ID')!

    arktest.assertEQ(childIdFld.getType(), Class.PRIMITIVE_NUMBER)
    arktest.assertEQ(childIdFld.getOwner(), childCls)
    arktest.assertNE(childCls.getStaticFieldByName('STR'), undefined)

    const parentFld = parentCls.getStaticFieldByName('STR')!
    const childFld = childCls.getStaticFieldByName('STR')!
    arktest.assertEQ(parentFld.getType(), childFld.getType())
    arktest.assertEQ(childCls.getStaticFieldByName('EOL')!.getType(), Class.PRIMITIVE_CHAR)
    arktest.assertTrue(parentFld.equals(childFld))
    arktest.assertFalse(parentIdFld.equals(childIdFld))

    arktest.assertEQ(parentCls.getStaticFieldByName('PROP'), undefined)
    arktest.assertEQ(childCls.getStaticFieldByName('PROP'), undefined)

    arktest.assertEQ(parentCls.getStaticFieldByName('PRIV'), undefined)
    arktest.assertEQ(childCls.getStaticFieldByName('PROT'), undefined)

    const parentFlds = parentCls.getStaticFields()
    const parentFldNames = new Set<string>(Array.from<reflect.StaticField, string>(parentFlds, (f) => f.getName()))
    arktest.assertFalse(parentFldNames.has('PRIV'))
    arktest.assertFalse(parentFldNames.has('PROT'))
    arktest.assertTrue(parentFldNames.has('ID'))

    const childFlds = childCls.getStaticFields()
    const childFldNames = new Set<string>(Array.from<reflect.StaticField, string>(childFlds, (f) => f.getName()))
    arktest.assertFalse(childFldNames.has('PROT'))
    arktest.assertTrue(childFldNames.has('ID'))
}

interface Iface {
    id: string
}

class IfaceImpl implements Iface {
    get id():string {return ''}
    set id(v: string) {}
}

function testInterfaceField(): void {
    const ifaces: FixedArray<Iface> = []
    const ifaceCls = Class.of(ifaces).getFixedArrayComponentType()!

    const idFld = ifaceCls.getInstanceFieldByName('id')
    arktest.assertEQ(idFld, undefined)

    const impl = new IfaceImpl()
    const idFldImpl = Class.of(impl).getInstanceFieldByName('id')
    arktest.assertEQ(idFldImpl, undefined)

    arktest.assertEQ(ifaceCls.getInstanceFields().length, 0)
    arktest.assertEQ(ifaceCls.getStaticFields().length, 0)
}

function main() {
    const suite = new arktest.ArkTestsuite('Reflect Field tests')
    suite.addTest('Test Reflect Field APIs', testField)
    suite.addTest('Test Reflect instance Field APIs', testInstanceField)
    suite.addTest('Test Reflect static Field APIs', testStaticField)
    suite.addTest('Test Reflect interface Field APIs', testInterfaceField)
    return suite.run()
}
