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
    arktest.assertEQ(xf.getType(), Class.of(1.0))
    arktest.assertFalse(xf.isReadonly())
    arktest.assertTrue(xf.isPublic())
    arktest.assertFalse(xf.isProtected())
    arktest.assertFalse(xf.isPrivate())
    arktest.assertTrue(isInherited(xf, typePoint3D))
    arktest.assertFalse(isOverrided(xf, typePoint3D))
    arktest.assertEQ(yf.getName(), "y")
    arktest.assertEQ(yf.getOwner(), typePoint3D)
    arktest.assertEQ(yf.getType(), Class.of(1.0))
    arktest.assertFalse(yf.isReadonly())
    arktest.assertTrue(yf.isPublic())
    arktest.assertFalse(yf.isProtected())
    arktest.assertFalse(yf.isPrivate())
    arktest.assertFalse(isInherited(yf, typePoint3D))
    arktest.assertTrue(isOverrided(yf, typePoint3D))
    arktest.assertEQ(sf.getName(), "state")
    arktest.assertEQ(sf.getOwner(), typePoint3D)
    arktest.assertEQ(sf.getType(), Class.of(1))
    arktest.assertFalse(sf.isReadonly())
    arktest.assertTrue(sf.isPublic())
    arktest.assertFalse(sf.isProtected())
    arktest.assertFalse(sf.isPrivate())
    arktest.assertFalse(isInherited(sf, typePoint3D))
    arktest.assertFalse(isOverrided(sf, typePoint3D))
    arktest.assertEQ(rf.getName(), "r")
    arktest.assertEQ(rf.getOwner(), typePoint3D)
    arktest.assertEQ(rf.getType(), Class.of(1))
    arktest.assertTrue(rf.isReadonly())
    arktest.assertTrue(rf.isPublic())
    arktest.assertFalse(rf.isProtected())
    arktest.assertFalse(rf.isPrivate())
    arktest.assertFalse(isInherited(rf, typePoint3D))
    arktest.assertFalse(isOverrided(rf, typePoint3D))
    arktest.assertEQ(abilityf.getName(), "ability")
    arktest.assertEQ(abilityf.getOwner(), typePoint3D)
}

class Parent {
    private static PRIV: boolean = true
    protected static PROT: string = 'prot'

    static ID: number = 1.0
    static EMPTY: string = ''

    static get PROP(): number {
        return 1
    }

    get prop(): number {
        return 0
    }

    id: number = 0
    private priv = true
    protected prot = true
}

class Child extends Parent {
    static ID: number = 2.0
    static EOL: char = c'\n'
    public EOL: string = '\n'

    id: number = 1
}

function testInstanceField(): void {
    const parentCls = Class.of(new Parent)
    const parentIdFld = parentCls.getInstanceFieldByName('id')!

    arktest.assertEQ(parentIdFld.getType(), Class.of(new Number(0)))
    arktest.assertEQ(parentIdFld.getOwner(), parentCls)

    const childCls = Class.of(new Child)
    const childIdFld = childCls.getInstanceFieldByName('id')!

    arktest.assertEQ(childIdFld.getType(), Class.of(new Number(0)))
    arktest.assertEQ(childIdFld.getOwner(), childCls)
    arktest.assertEQ(childIdFld.getType(), parentIdFld.getType())

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
    const parentCls = Class.of(new Parent)
    const parentIdFld = parentCls.getStaticFieldByName('ID')!

    arktest.assertEQ(parentIdFld.getType(), Class.of(new Number(0)))
    arktest.assertEQ(parentIdFld.getOwner(), parentCls)
    arktest.assertEQ(parentCls.getStaticFieldByName('EOL'), undefined)

    const childCls = Class.of(new Child)
    const childIdFld = childCls.getStaticFieldByName('ID')!

    arktest.assertEQ(childIdFld.getType(), Class.of(new Number(0)))
    arktest.assertEQ(childIdFld.getOwner(), childCls)
    arktest.assertNE(childCls.getStaticFieldByName('EMPTY'), undefined)

    const parentFld = parentCls.getStaticFieldByName('EMPTY')!
    const childFld = childCls.getStaticFieldByName('EMPTY')!
    arktest.assertEQ(parentFld.getType(), childFld.getType())
    arktest.assertEQ(childCls.getStaticFieldByName('EOL')!.getType(), Class.of(new Char(c'\n')))
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

function testInterfaceField(): void {
    const ifaces: FixedArray<Iface> = []
    const ifaceCls = Class.of(ifaces).getFixedArrayComponentType()!

    const idFld = ifaceCls.getInstanceFieldByName('id')
    arktest.assertEQ(idFld, undefined)

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
