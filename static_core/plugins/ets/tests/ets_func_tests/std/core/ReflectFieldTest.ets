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

function main() {
    const suite = new arktest.ArkTestsuite('Reflect Field tests')
    suite.addTest('Test Reflect Field APIs', testField)
    return suite.run()
}
