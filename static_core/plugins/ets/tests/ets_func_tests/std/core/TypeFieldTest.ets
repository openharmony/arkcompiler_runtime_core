/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

function test(result: boolean, name: String): int {
    if (result) {
        console.println("PASSED: " + name)
        return 0
    }
    console.println("FAILED: " + name)
    return 1
}

interface Serializable {
    serialize(): String;
}

class Point implements Serializable {
    protected x: double
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
    private state: int
    static ZERO: double = 0.0
}

function testField(): int {
    let pt = Type.of(new Point3D()) as ClassType

    let xf = pt.getFieldByName("x")
    let sf = pt.getFieldByName("state")
    let zf = pt.getFieldByName("ZERO")

    return test(xf.getName() == "x",                                        "FieldName: x") +
            test(xf.getOwnerType().equals(Type.of(new Point())),            "   OwnerType: Point") +
            test(xf.getType() == DoubleType.VAL,                            "   Type: double") +
            test(xf.getAttributes() == Attributes.INHERITED,                "   no attributes") +
            test(xf.getAccessModifier() == AccessModifier.PROTECTED,        "   protected access modififer") +
            test(xf.isInherited(),                                          "   inherited") +
            test(!xf.isStatic(),                                            "   not static") +
            test(sf.getName() == "state",                                   "FieldName: state") +
            test(sf.getOwnerType().equals(pt),                              "   OwnerType: Point3D") +
            test(sf.getType() == IntType.VAL,                               "   Type: int") +
            test(sf.getAttributes() == 0,                                   "   no attributes") +
            test(sf.getAccessModifier() == AccessModifier.PRIVATE,          "   private access modifier") +
            test(!sf.isInherited(),                                         "   not inherited") +
            test(!sf.isStatic(),                                            "   not static") +
            test(zf.getName() == "ZERO",                                    "FieldName: ZERO") +
            test(zf.getOwnerType().equals(pt),                              "   OwnerType: Point3D") +
            test(zf.getType() == DoubleType.VAL,                            "   Type: double") +
            test(zf.getAttributes() == 1,                                   "   static attribute") +
            test(zf.getAccessModifier() == AccessModifier.PUBLIC,           "   public access modififer") +
            test((zf.getStaticValue() as Double).unboxed() == Point3D.ZERO, "   static value") +
            test(!zf.isInherited(),                                         "   not inherited") +
            test(zf.isStatic(),                                             "   static")
}

function main(): int {
    let failures = 0
    failures += testField()
    if (failures == 0) {
        console.println("PASSED: All tests run")
    } else {
        console.println("FAILED: All tests run")
    }
    return failures
}
