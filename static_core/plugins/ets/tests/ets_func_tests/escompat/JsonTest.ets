/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

function testNull(): int {
    let nullType = Type.of(null)
    return test(JSON.parse<Object>("null", nullType) == null, "Null")
}

function testBoolean(): int {
    let booleanType = Type.of(new Boolean() as Object)
    return test((JSON.parse<Object>("true", booleanType) as Boolean).unboxed() == true, "Boolean(true)") +
          test((JSON.parse<Object>("false", booleanType) as Boolean).unboxed() == false, "Boolean(false)")
}

function testDouble(): int {
    let doubleType = Type.of(new Double() as Object)
    return test((JSON.parse<Object>("0", doubleType) as Double).unboxed() == 0, "Double(0)") +
        test((JSON.parse<Object>("1", doubleType) as Double).unboxed() == 1, "Double(1)") +
        test((JSON.parse<Object>("-1", doubleType) as Double).unboxed() == -1, "Double(-1)") +
        test((JSON.parse<Object>("2.72182", doubleType) as Double).unboxed() == 2.72182, "Double(2.72182)") +
        test((JSON.parse<Object>("-2.72182", doubleType) as Double).unboxed() == -2.72182, "Double(-2.72182)")
}

function testString(): int {
    let stringType = Type.of(new String() as Object)
    return test(JSON.parse<Object>("\"Gracias\"", stringType) as String == "Gracias", "String(Gracias)") +
        test(JSON.parse<Object>("\"\"", stringType) as String == "", "String()")
}

function equalObjectArrays(a: Object[], b: Object[]): boolean {
    if (a.length != b.length) {
        return false
    }
    for (let i = 0; i < a.length; i++) {
        if (a[i] != b[i]) {
            return false
        }
    }
    return true
}

function testArrayBoolean(): int {
    let booleanType = Type.of(new Boolean[1] as Object)
    let emptyArray : Boolean[] = []
    return test(equalObjectArrays(JSON.parse<Object>("[false, true]", booleanType) as Boolean[],
            [Boolean.valueOf(false), Boolean.valueOf(true)]), "[false, true]") +
        test(equalObjectArrays(JSON.parse<Object>("[]", booleanType) as Boolean[], emptyArray), "[]")
}

function testArrayDouble(): int {
    let doubleType = Type.of(new Double[1] as Object)
    let emptyArray : Double[] = []
    return test(equalObjectArrays(JSON.parse<Object>("[0, -1, 1, 2.72182, -2.72182]", doubleType) as Double[],
            [Double.valueOf(0), Double.valueOf(-1), Double.valueOf(1), Double.valueOf(2.72182), Double.valueOf(-2.72182)]), "[0, -1, 1, 2.72182, -2.72182]") +
        test(equalObjectArrays(JSON.parse<Object>("[]", doubleType) as Double[], emptyArray), "[]")
}

function testArrayString(): int {
    let stringType = Type.of(new String[1] as Object)
    let emptyArray : String[] = []
    return test(equalObjectArrays(JSON.parse<Object>("[\"White\", \"Rabbit\", \"was running away\", \"from Alice\"]", stringType) as String[],
            ["White", "Rabbit", "was running away", "from Alice"]), "[\"White\", \"Rabbit\", \"was running away\", \"from Alice\"]") +
        test(equalObjectArrays(JSON.parse<Object>("[]", stringType) as String[], emptyArray), "[]")
}

function testObject(): int {
    let objectType = Type.of(new Object())
    return test(JSON.stringify(JSON.parse<Object>("{}", objectType) as Object) == "{}", "Object")
}

export class User {}

class UserWithFields extends User {
    id: Double
    g: Boolean
    name: String
}

class SuperUser extends UserWithFields {
    users: UserWithFields[]
}

function testStringifyParse(): int {
    let classType = Type.of(new SuperUser() as Object)
    let str = "{\"id\":10,\"g\":true,\"name\":\"Name\",\"users\":[{\"id\":12,\"g\":true,\"name\":\"Name1\"},{\"id\":13,\"g\":false,\"name\":\"Name2\"}]}";
    return test(JSON.stringify(JSON.parse<SuperUser>(str, classType) as SuperUser) == str, "SuperUser")
}

class NoConstructor {
    x : double
    constructor(x : double) {
        this.x = x
    }
}

class ForbiddenPrimitives {
    x : int
}

class CycleA {
    x : CycleB
}

class CycleB {
    x : CycleA
}

function testCheckParse(): int {
    let errors = 0
    errors += test(!JSON.isCorrectForParsing(Type.of(new NoConstructor(10.0) as Object)), "No constructor")
    errors += test(!JSON.isCorrectForParsing(Type.of(new ForbiddenPrimitives() as Object)), "Forbidden primitives")
    errors += test(!JSON.isCorrectForParsing(Type.of(new CycleA() as Object)), "Cycle")
    return errors
}

function main(): int {
    let failures = 0
    failures += testNull()
    failures += testBoolean()
    failures += testDouble()
    failures += testString()
    failures += testArrayBoolean()
    failures += testArrayDouble()
    failures += testArrayString()
    failures += testObject()
    failures += testStringifyParse()
    failures += testCheckParse()
    if (failures == 0) {
        console.println("PASSED: All tests run")
    } else {
        console.println("FAILED: All tests run")
    }
    return failures
}
