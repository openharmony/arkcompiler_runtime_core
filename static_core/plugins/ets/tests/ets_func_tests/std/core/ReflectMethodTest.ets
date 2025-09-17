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

    testUnion(a: Int | Int[]): String | Int {
        if (a instanceof Int) {
            return "1"
        }
        return 2
    }

    optionalParams(x: int, opt1: int = 1, opt2: int = 2) {}
    restParams(x: int[], ...rest: int[]) {}
    optReadonlyRestParams(x: readonly int[] = [4], y: int = 4, z: readonly [number, string] = [1, "123"], ...rest: int[]) {}
}

abstract class Writer {
    abstract write(): Int
    close() {}
}

class HTMLWriter extends Writer {
    private state_: int

    get state(): int {
        return this.state_
    }

    set state(state: int) {
        this.state_ = state
    }

    static createFileExt(file: String): String {
        return file + ".html"
    }

    override write(): Int {
        return new Int(2)
    }

    constructor(state: int) {
        this.state_ = state
    }

    final finalMethod() {}

    native nativeMethod(): boolean

    async asyncMethod() {}
}

function testReflectMethod(): void {
    let typePoint = Class.of(new Point())
    let typeHWriter = Class.of(new HTMLWriter(10))
    let typeWriter = typeHWriter.getSuper()!

    let serializeMethod = typePoint.getInstanceMethodByName("serialize")!
    let addMethod = typePoint.getInstanceMethodByName("add")!
    let unionMethod = typePoint.getInstanceMethodByName("testUnion")!
    let getter = typeHWriter.getInstanceMethodByName("<get>state")!
    let setter = typeHWriter.getInstanceMethodByName("<set>state")!

    arktest.assertEQ(serializeMethod.getReturnType(), Class.of(""))
    arktest.assertEQ(serializeMethod.getOwner(), typePoint)
    arktest.assertEQ(serializeMethod.getParametersNum(), 0)
    arktest.assertEQ(addMethod.getParametersNum(), 1)
    arktest.assertEQ(addMethod.getParameterType(0)!, typePoint)
    arktest.assertEQ(addMethod.getReturnType(), typePoint)
    arktest.assertEQ(addMethod.getOwner(), typePoint)
    arktest.assertEQ(getter.getParametersNum(), 0)
    arktest.assertEQ(getter.getReturnType(), Class.of(1))
    arktest.assertEQ(getter.getOwner(), typeHWriter)
    arktest.assertEQ(setter.getParametersNum(), 1)
    arktest.assertEQ(setter.getParameterType(0)!, Class.of(1))
    arktest.assertEQ(setter.getReturnType().getName(), "void")
    arktest.assertEQ(setter.getOwner(), typeHWriter)
}

function testMethodAttributes(): void {
    let typePoint = Class.of(new Point())
    let typeHWriter = Class.of(new HTMLWriter(10))
    let typeWriter = typeHWriter.getSuper()!

    let getterMethod = typeHWriter.getInstanceMethodByName("<get>state")!
    let setterMethod = typeHWriter.getInstanceMethodByName("<set>state")!
    let abstractWriteMethod = typeWriter.getInstanceMethodByName("write")!
    let overrideWriteMethod = typeHWriter.getInstanceMethodByName("write")!
    let closeMethod = typeHWriter.getInstanceMethodByName("close")!
    let finalMethod = typeHWriter.getInstanceMethodByName("finalMethod")!
    let nativeMethod = typeHWriter.getInstanceMethodByName("nativeMethod")!
    let asyncMethod = typeHWriter.getInstanceMethodByName("asyncMethod")!
    let constructors = typeHWriter.getConstructors()
    let constructorOfClass = constructors[0]
 
    let isInherited = (m: reflect.InstanceMethod, type: Class): boolean => {
        return m.getOwner().getName() != type.getName()
    }
    let isOverrided = (m: reflect.InstanceMethod, type: Class): boolean => {
        if (m.getOwner().getName() != type.getName()) {
            return false
        }
        let baseType = type.getSuper()!
        return baseType.getInstanceMethodByName(m.getName()) != undefined
    }

    arktest.assertEQ(getterMethod.getName(), "<get>state")
    arktest.assertFalse(isInherited(getterMethod, typeHWriter))
    arktest.assertFalse(isOverrided(getterMethod, typeHWriter))
    arktest.assertFalse(getterMethod.isAbstract())
    arktest.assertTrue(getterMethod.isGetter())
    arktest.assertFalse(getterMethod.isSetter())
    arktest.assertFalse(getterMethod.isFinal())
    arktest.assertFalse(getterMethod.isNative())
    arktest.assertFalse(getterMethod.isAsync())
    arktest.assertTrue(getterMethod.isPublic())
    arktest.assertFalse(getterMethod.isProtected())
    arktest.assertFalse(getterMethod.isPrivate())
    arktest.assertEQ(setterMethod.getName(), "<set>state")
    arktest.assertFalse(isInherited(setterMethod, typeHWriter))
    arktest.assertFalse(isOverrided(setterMethod, typeHWriter))
    arktest.assertFalse(setterMethod.isAbstract())
    arktest.assertFalse(setterMethod.isGetter())
    arktest.assertTrue(setterMethod.isSetter())
    arktest.assertFalse(setterMethod.isFinal())
    arktest.assertFalse(setterMethod.isNative())
    arktest.assertFalse(setterMethod.isAsync())
    arktest.assertTrue(setterMethod.isPublic())
    arktest.assertFalse(setterMethod.isProtected())
    arktest.assertFalse(setterMethod.isPrivate())
    arktest.assertEQ(overrideWriteMethod.getName(), "write")
    arktest.assertFalse(isInherited(overrideWriteMethod, typeHWriter))
    arktest.assertTrue(isOverrided(overrideWriteMethod, typeHWriter))
    arktest.assertFalse(overrideWriteMethod.isAbstract())
    arktest.assertFalse(overrideWriteMethod.isGetter())
    arktest.assertFalse(overrideWriteMethod.isSetter())
    arktest.assertFalse(overrideWriteMethod.isFinal())
    arktest.assertFalse(overrideWriteMethod.isNative())
    arktest.assertFalse(overrideWriteMethod.isAsync())
    arktest.assertTrue(overrideWriteMethod.isPublic())
    arktest.assertFalse(overrideWriteMethod.isProtected())
    arktest.assertFalse(overrideWriteMethod.isPrivate())
    arktest.assertEQ(abstractWriteMethod.getName(), "write")
    arktest.assertFalse(isInherited(abstractWriteMethod, typeWriter))
    arktest.assertFalse(isOverrided(abstractWriteMethod, typeWriter))
    arktest.assertTrue(abstractWriteMethod.isAbstract())
    arktest.assertFalse(abstractWriteMethod.isGetter())
    arktest.assertFalse(abstractWriteMethod.isSetter())
    arktest.assertFalse(abstractWriteMethod.isFinal())
    arktest.assertFalse(abstractWriteMethod.isNative())
    arktest.assertFalse(abstractWriteMethod.isAsync())
    arktest.assertTrue(abstractWriteMethod.isPublic())
    arktest.assertFalse(abstractWriteMethod.isProtected())
    arktest.assertFalse(abstractWriteMethod.isPrivate())
    arktest.assertEQ(closeMethod.getName(), "close")
    arktest.assertTrue(isInherited(closeMethod, typeHWriter))
    arktest.assertFalse(isOverrided(closeMethod, typeHWriter))
    arktest.assertFalse(closeMethod.isAbstract())
    arktest.assertFalse(closeMethod.isGetter())
    arktest.assertFalse(closeMethod.isSetter())
    arktest.assertFalse(closeMethod.isFinal())
    arktest.assertFalse(closeMethod.isNative())
    arktest.assertFalse(closeMethod.isAsync())
    arktest.assertTrue(closeMethod.isPublic())
    arktest.assertFalse(closeMethod.isProtected())
    arktest.assertFalse(closeMethod.isPrivate())
    arktest.assertEQ(finalMethod.getName(), "finalMethod")
    arktest.assertFalse(isInherited(finalMethod, typeHWriter))
    arktest.assertFalse(isOverrided(finalMethod, typeHWriter))
    arktest.assertFalse(finalMethod.isAbstract())
    arktest.assertFalse(finalMethod.isGetter())
    arktest.assertFalse(finalMethod.isSetter())
    arktest.assertTrue(finalMethod.isFinal())
    arktest.assertFalse(finalMethod.isNative())
    arktest.assertFalse(finalMethod.isAsync())
    arktest.assertTrue(finalMethod.isPublic())
    arktest.assertFalse(finalMethod.isProtected())
    arktest.assertFalse(finalMethod.isPrivate())
    arktest.assertEQ(nativeMethod.getName(), "nativeMethod")
    arktest.assertFalse(isInherited(nativeMethod, typeHWriter))
    arktest.assertFalse(isOverrided(nativeMethod, typeHWriter))
    arktest.assertFalse(nativeMethod.isAbstract())
    arktest.assertFalse(nativeMethod.isGetter())
    arktest.assertFalse(nativeMethod.isSetter())
    arktest.assertFalse(nativeMethod.isFinal())
    arktest.assertTrue(nativeMethod.isNative())
    arktest.assertFalse(nativeMethod.isAsync())
    arktest.assertTrue(nativeMethod.isPublic())
    arktest.assertFalse(nativeMethod.isProtected())
    arktest.assertFalse(nativeMethod.isPrivate())
    arktest.assertEQ(asyncMethod.getName(), "asyncMethod")
    arktest.assertFalse(isInherited(asyncMethod, typeHWriter))
    arktest.assertFalse(isOverrided(asyncMethod, typeHWriter))
    arktest.assertFalse(asyncMethod.isAbstract())
    arktest.assertFalse(asyncMethod.isGetter())
    arktest.assertFalse(asyncMethod.isSetter())
    arktest.assertFalse(asyncMethod.isFinal())
    arktest.assertFalse(asyncMethod.isNative())
    arktest.assertTrue(asyncMethod.isAsync())
    arktest.assertTrue(asyncMethod.isPublic())
    arktest.assertFalse(asyncMethod.isProtected())
    arktest.assertFalse(asyncMethod.isPrivate())
    arktest.assertEQ(constructorOfClass.getName(), "constructor")
    arktest.assertFalse(constructorOfClass.isNative())
    arktest.assertTrue(constructorOfClass.isPublic())
    arktest.assertFalse(constructorOfClass.isProtected())
    arktest.assertFalse(constructorOfClass.isPrivate())
}

function testParameters() {
    let typePoint = Class.of(new Point())

    let optionalParamsMethod = typePoint.getInstanceMethodByName("optionalParams")!
    let restParamsMethod = typePoint.getInstanceMethodByName("restParams")!
    let optReadonlyRestParamsMethod = typePoint.getInstanceMethodByName("optReadonlyRestParams")!

    let res = 0
    let paramsOpt = optionalParamsMethod.getParametersTypes()
    let paramsRest = restParamsMethod.getParametersTypes()
    arktest.assertEQ(optionalParamsMethod.getParametersNum(), 3)
    arktest.assertEQ(restParamsMethod.getParametersNum(), 2)
    arktest.assertEQ(optReadonlyRestParamsMethod.getParametersNum(), 4)
}

function main() {
    const suite = new arktest.ArkTestsuite('Reflect Method tests')
    suite.addTest('Test Reflect Method base APIs', testReflectMethod)
    suite.addTest('Test Reflect Method attributes APIs', testMethodAttributes)
    suite.addTest('Test Reflect Method Parameters APIs', testParameters)
    return suite.run()
}
