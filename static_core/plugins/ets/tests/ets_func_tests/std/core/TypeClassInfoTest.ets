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
    public foo1(): void {}
    public foo2(): void {}

    public static stfoo1(): void {}
    public static stfoo2(): void {}

    private prfoo1(): void {}
    private prfoo2(): void {}
}

class B extends A {
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

function testGetName(typeInfo: reflect.TypeInfo, expected: string) {
    arktest.assertEQ(typeInfo.getName(), expected);
}

function testGetInstanceMethods(classTypeInfo: reflect.ClassTypeInfo, expectedMethodsArr: string[]) {
    let expectedMtdsSet: Set<string> = new Set<string>(expectedMethodsArr);

    let methods: FixedArray<Method> = classTypeInfo.getInstanceMethods()
    arktest.assertEQ(methods.length, expectedMethodsArr.length)

    for (let i = 0; i < methods.length; ++i) {
        arktest.assertTrue(expectedMtdsSet.has(methods[i].getName()))
    }
}

function testGetInstanceMethodByName(classTypeInfo: reflect.ClassTypeInfo, methodsToFind: string[]) {
    for (let i = 0; i < methodsToFind.length; ++i) {
        let obtainedMethod = classTypeInfo.getInstanceMethodByName(methodsToFind[i])

        if (obtainedMethod == undefined) {
            throw new Error(`Can't find given method '${methodsToFind[i]}' in testGetInstanceMethodByName`)
        }

        arktest.assertEQ(obtainedMethod.getName(), methodsToFind[i])
    }
}

function tryGetPrivateInstanceMethodByName(classTypeInfo: reflect.ClassTypeInfo, methods: string[]) {
    for (let i = 0; i < methods.length; ++i) {
        let obtainedMethod = classTypeInfo.getInstanceMethodByName(methods[i])
        if (obtainedMethod != undefined) {
            throw new Error(`Given method '${methods[i]}' in tryGetPrivateInstanceMethodByName should be undefined`)
        }
    }
}

function main(): int {
    let baseObject = Class.of(new Object())
    let classTypeInfoBaseObj = reflect.TypeInfo.of(baseObject) as reflect.ClassTypeInfo

    testGetName(classTypeInfoBaseObj, "std.core.Object")

    let classA = Class.of(new A())
    let typeInfoA = reflect.TypeInfo.of(classA)
    let classTypeInfoA: reflect.ClassTypeInfo = typeInfoA as reflect.ClassTypeInfo

    testGetName(typeInfoA, "TypeClassInfoTest.A")
    testGetName(classTypeInfoA, "TypeClassInfoTest.A")

    let classB = Class.of(new B())
    let typeInfoB = reflect.TypeInfo.of(classB)
    let classTypeInfoB: reflect.ClassTypeInfo = typeInfoB as reflect.ClassTypeInfo

    testGetName(typeInfoB, "TypeClassInfoTest.B")
    testGetName(classTypeInfoB, "TypeClassInfoTest.B")

    // Test getBase()
    arktest.assertEQ(classTypeInfoBaseObj.getBase(), undefined)
    arktest.assertEQ(classTypeInfoA.getBase(), baseObject)
    arktest.assertEQ(classTypeInfoB.getBase(), classA)

    // Test methods getters
    let baseObjInstanceMethodsExpected =
        [ "toString", "toLocaleString", "$_hashCode", "hasOwnProperty", "hasOwnProperty" ]

    let baseObjStaticMethodsExpected =
        [ "keys", "keys", "assign", "hasOwn", "hasOwn", "values", "values", "entries",
          "entries", "fromEntries", "getOwnPropertyNames", "getOwnPropertyNames" ]

    testGetInstanceMethods(classTypeInfoBaseObj, baseObjInstanceMethodsExpected)
    testGetInstanceMethods(classTypeInfoA, baseObjInstanceMethodsExpected.concat(["someDefaultMethod", "foo1", "foo2"]))
    testGetInstanceMethods(classTypeInfoB, baseObjInstanceMethodsExpected.concat(["someDefaultMethod", "foo1", "foo2", "boo1", "boo2"]))

    testGetInstanceMethodByName(classTypeInfoBaseObj, baseObjInstanceMethodsExpected)
    testGetInstanceMethodByName(classTypeInfoA, baseObjInstanceMethodsExpected.concat(["someDefaultMethod", "foo1", "foo2"]))
    testGetInstanceMethodByName(classTypeInfoB, baseObjInstanceMethodsExpected.concat(["someDefaultMethod", "foo1", "foo2", "boo1", "boo2"]))

    tryGetPrivateInstanceMethodByName(classTypeInfoA, [ "prfoo1", "prfoo2" ])

    return 0
}
