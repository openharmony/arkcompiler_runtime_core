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

    constructor(x: double, y: double) {
        this.x = x
        this.y = y
    }

    constructor() {
        this(0.0, 0.0)
    }

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
    constructor(x: double, y: double, z: double) {
        super(x, y)
        this.z = z
        this.state = 0.0
    }
    constructor(state: int) {
        this(0, 0, 0)
        this.state = 1.0
    }
}

abstract class Writer {
    abstract write(): int
}

final class HTMLWriter extends Writer {
    private state: int
    constructor(state: int) {
        this.state = state
    }

    public getState(): int {
        return this.state
    }

    override write(): int {
        return 0
    }
}

function findMethodByName(ct: ClassType, name: string): Method | null {
    for (let i = 0; i < ct.getMethodsNum(); ++i) {
        let method = ct.getMethod(i)
        if (method.getName() == name) {
            return method
        }
    }
    return null
}

function testFields(ct: ClassType): int {
    console.log("        Fields:")
    let failures = 0
    for (let i = 0; i < ct.getFieldsNum(); ++i) {
        let field = ct.getField(i);
        failures += test(ct.getFieldByName(field.getName()).equals(field), "   " + field.getName() + " field")
    }
    return failures
}

function testAllFieldsAreOwn(ct: ClassType): int {
    console.log("         Own Fields:")
    let failures = test(ct.getOwnFieldsNum() == ct.getFieldsNum(), "   OwnFieldsNum == FieldsNum")
    for (let i = 0; i < ct.getOwnFieldsNum(); ++i) {
        let field = ct.getOwnField(i);
        failures += test(ct.getFieldByName(field.getName()).equals(field), "  " + field.getName())
    }
    return failures
}

function testClassExtendsObject(ct: ClassType): int {
    let failures = 0
    let objectMethods = ObjectType.getMethodsNum()
    for (let i = 0; i < objectMethods; ++i) {
        let objectMethodName = ObjectType.getMethod(i).getName()
        failures += test(findMethodByName(ct, objectMethodName) != null,  "       " + objectMethodName + " method")
    }
    return failures
}

function testPoint(): int {
    let pt = Type.of(new Point()) as ClassType

    return test(pt.getName() == "Point",                                    "ClassName: Point") +
            test(pt.hasEmptyConstructor(),                                  "   doesn't have empty constuctor") +
            test(pt.getBaseType().equals(ObjectType),                       "   base type is ObjectType") +
            test(pt.getInterfacesNum() == 1,                                "   1 Interface:") +
            test(pt.getInterface(0).getName() == "Serializable",            "        Serializable interface") +
            test(pt.getFieldsNum() == 2,                                    "   2 Fields:") +
            test(pt.hasField("x"),                                          "       x field") +
            test(pt.hasField("y"),                                          "       y field") +
            testAllFieldsAreOwn(pt) +
            test(pt.getMethodsNum() == 2 + ObjectType.getMethodsNum(),      "   2 Methods: ") +
            testClassExtendsObject(pt) +
            test(findMethodByName(pt, "add") != null,                       "       Point.add method") +
            test(findMethodByName(pt, "serialize") != null,                 "       Serializable.serialize method") +
            test(pt.getConstructorsNum() == 2,                              "   2 Constructor:") +
            test(pt.getConstructor(0).getName() == "constructor",           "       default") +
            test(pt.getConstructor(1).getName() == "constructor",           "       default") +
            test(!pt.isFinal(),                                             "   not final") +
            testFields(pt)
}

function testPoint3D(): int {
    let pt = Type.of(new Point3D(1.0, 2.0, 3.0)) as ClassType

    return test(pt.getName() == "Point3D",                                  "ClassName: Point3D") +
            test(!pt.hasEmptyConstructor(),                                 "   doesn't have empty constructor") +
            test(pt.getBaseType().getName() == "Point",                     "   base type is Point") +
            test(pt.getInterfacesNum() == 0,                                "   0 Interfaces") +
            test(pt.getFieldsNum() == 5,                                    "   5 Fields:") +
            test(pt.hasField("x"),                                          "       x field") +
            test(pt.hasField("y"),                                          "       y field") +
            test(pt.hasField("z"),                                          "       z field") +
            test(pt.hasField("state"),                                      "       state field") +
            test(pt.hasField("ZERO"),                                       "       ZERO field") +
            test(pt.getOwnFieldsNum() == 3,                                 "   3 Own Fields:") +
            test(pt.getOwnField(0).equals(pt.getFieldByName("ZERO")),       "       ZERO field") +
            test(pt.getOwnField(1).equals(pt.getFieldByName("z")),          "       z field") +
            test(pt.getOwnField(2).equals(pt.getFieldByName("state")),      "       state field") +
            test(pt.getMethodsNum() == 2 + ObjectType.getMethodsNum(),      "   2 Methods:") +
            testClassExtendsObject(pt) +
            test(findMethodByName(pt, "add") != null,                       "       Point.add method") +
            test(findMethodByName(pt, "serialize") != null,                 "       Serializable.serialize method") +
            test(pt.getConstructorsNum() == 2,                              "   2 Constructors") +
            testFields(pt)
}

function testHTMLWriter(): int {
    let wt = Type.of(new HTMLWriter(1)) as ClassType

    return test(wt.getName() == "HTMLWriter",                               "ClassName: HTMLWriter") +
            test(!wt.hasEmptyConstructor(),                                 "   doesn't have empty constructor") +
            test(wt.getBaseType().getName() == "Writer",                    "   base type is Writer") +
            test(wt.getInterfacesNum() == 0,                                "   0 Interfaces") +
            test(wt.getFieldsNum() == 1,                                    "   1 Field:") +
            test(wt.hasField("state"),                                      "       state field") +
            testAllFieldsAreOwn(wt) +
            test(wt.getMethodsNum() == 2 + ObjectType.getMethodsNum(),      "   5 methods:") +
            testClassExtendsObject(wt) +
            test(findMethodByName(wt, "write") != null,                     "       HTMLWriter.add method") +
            test(findMethodByName(wt, "getState") != null,                  "       HTMLWriter.getState method") +
            test(wt.getConstructorsNum() == 1,                              "   1 Constructor:") +
            test(wt.getConstructor(0).getName() == "constructor",           "       default") +
            test(wt.isFinal(),                                              "   final") +
            testFields(wt)
}

function testMake(): int {
    console.log("        Make:")
    let failures = 0
    let pt = Type.of(new Point()) as ClassType
    let wt = Type.of(new HTMLWriter(0)) as ClassType
    let p3t = Type.of(new Point3D(10)) as ClassType
    let point = pt.make([new Double(1.0), new Double(2.0)] as NullishType[]) as Point
    let point3D = p3t.make([new Double(1.0), new Double(2.0), new Double(3.0)] as NullishType[]) as Point3D
    let writer = wt.make([new Int(10)] as NullishType[]) as HTMLWriter
    failures += test(point.serialize() == "{1,2}", "Point")
    failures += test(point3D.serialize() == "{1,2}" && point3D.z == 3, "Point3D")
    failures += test(writer.getState() == 10, "HTMLWriter" )
    return failures
}

function testClassType(): int {
    let failures = 0
    failures += testPoint()
    failures += testPoint3D()
    failures += testHTMLWriter()
    failures += testMake()
    return failures
}

function main(): int {
    let failures = 0
    failures += testClassType()
    if (failures == 0) {
        console.println("PASSED: All tests run")
    } else {
        console.println("FAILED: All tests run")
    }
    return failures
}
