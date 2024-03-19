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

abstract class Writer {
    abstract write(): Int;
    protected close() {}
}

class HTMLWriter extends Writer {
    private state_: int

    get state(): int {
        return this.state_
    }

    private set state(state: int) {
        this.state_ = state;
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

function testMethodType(): int {
    let pt = Type.of(new Point()) as ClassType
    let hwt = Type.of(new HTMLWriter(10)) as ClassType

    let serializeMethodType = findMethodByName(pt, "serialize")!.getType()
    let addMethodType = findMethodByName(pt, "add")!.getType()
    let cType = hwt.getConstructor(0).getType()
    let getterType = findMethodByName(hwt, "<get>state")!.getType()
    let setterType = findMethodByName(hwt, "<set>state")!.getType()

    return  test(serializeMethodType.getParametersNum() == 0,           "   0 Parameters") +
            test(serializeMethodType.getResultType() == StringType.REF, "   has String result type") +
            test(serializeMethodType.getReceiverType().equals(pt),      "   has Point receiver type") +
            test(addMethodType.getParametersNum() == 1,                 "   1 parameter:") +
            test(addMethodType.getParameter(0).getType().equals(pt),    "       Point parameter") +
            test(addMethodType.getResultType().equals(pt),              "   has Point result type") +
            test(addMethodType.getReceiverType().equals(pt),            "   has Point receiver type") +
            test(cType.getParametersNum() == 1,                         "   1 Parameter") +
            test(cType.getParameter(0).getType() == IntType.VAL,        "       int parameter") +
            test(cType.getResultType() == VoidType.REF,                 "   has void result type") +
            test(cType.getReceiverType().equals(hwt),                   "   has HTMLWriter receiver type") +
            test(getterType.getParametersNum() == 0,                    "   0 Parameters") +
            test(getterType.getResultType() == IntType.VAL,             "   has int result type") +
            test(getterType.getReceiverType().equals(hwt),              "   has HTMLWriter receiver type") +
            test(setterType.getParametersNum() == 1,                    "   1 Parameter") +
            test(setterType.getParameter(0).getType() == IntType.VAL,   "       int parameter") +
            test(setterType.getResultType() == VoidType.REF,            "   has void result type") +
            test(setterType.getReceiverType().equals(hwt),              "   has HTMLWriter receiver type")
}

function testMethod() {
    let pt = Type.of(new Point()) as ClassType
    let hwt = Type.of(new HTMLWriter(10)) as ClassType
    let wt = hwt.getBaseType()

    let cMethod = hwt.getConstructor(0)
    let getterMethod = findMethodByName(hwt, "<get>state")!
    let setterMethod = findMethodByName(hwt, "<set>state")!
    let createFileExtMethod = findMethodByName(hwt, "createFileExt")!
    let abstractWriteMethod = findMethodByName(wt, "write")!
    let closeMethod = findMethodByName(hwt, "close")!

    return test(cMethod.getName() == "constructor",                            "MethodName: constructor") +
            test(cMethod.getAttributes() == Attributes.CONSTRUCTOR,             "   constructor attrib") +
            test(cMethod.getAccessModifier() == AccessModifier.PUBLIC,          "   public access modifier") +
            test(!cMethod.isInherited(),                                        "   not inherited") +
            test(!cMethod.isStatic(),                                           "   not static") +
            test(!cMethod.isAbstract(),                                         "   not abstract") +
            test(!cMethod.isGetter(),                                           "   not getter") +
            test(!cMethod.isSetter(),                                           "   not setter") +
            test(getterMethod.getName() == "<get>state",                        "MethodName: <get>state") +
            test(getterMethod.getAttributes() == Attributes.GETTER,             "   getter attrib") +
            test(getterMethod.getAccessModifier() == AccessModifier.PUBLIC,     "   public access modifier") +
            test(!getterMethod.isInherited(),                                   "   not inherited") +
            test(!getterMethod.isStatic(),                                      "   not static") +
            test(!getterMethod.isAbstract(),                                    "   not abstract") +
            test(getterMethod.isGetter(),                                       "   getter") +
            test(!getterMethod.isSetter(),                                      "   not setter") +
            test(setterMethod.getName() == "<set>state",                        "MethodName: <set>state") +
            test(setterMethod.getAttributes() == Attributes.SETTER,             "   setter attrib") +
            test(setterMethod.getAccessModifier() == AccessModifier.PRIVATE,    "   private access modifier") +
            test(!setterMethod.isInherited(),                                   "   not inherited") +
            test(!setterMethod.isStatic(),                                      "   not static") +
            test(!setterMethod.isAbstract(),                                    "   not abstract") +
            test(!setterMethod.isGetter(),                                      "   not getter") +
            test(setterMethod.isSetter(),                                       "   setter") +
            test(createFileExtMethod.getName() == "createFileExt",                     "MethodName: createFileExt") +
            test(createFileExtMethod.getAttributes() == Attributes.STATIC,             "   static attrib") +
            test(createFileExtMethod.getAccessModifier() == AccessModifier.PUBLIC,     "   public access modifier") +
            test(!createFileExtMethod.isInherited(),                                   "   not inherited") +
            test(createFileExtMethod.isStatic(),                                       "   static") +
            test(!createFileExtMethod.isAbstract(),                                    "   not abstract") +
            test(!createFileExtMethod.isGetter(),                                      "   not getter") +
            test(!createFileExtMethod.isSetter(),                                      "   not setter") +
            test(abstractWriteMethod.getName() == "write",                             "MethodName: write") +
            test(abstractWriteMethod.getAttributes() == Attributes.ABSTRACT,           "   abstract attrib") +
            test(abstractWriteMethod.getAccessModifier() == AccessModifier.PUBLIC,     "   public access modifier") +
            test(!abstractWriteMethod.isInherited(),                                   "   not inherited") +
            test(!abstractWriteMethod.isStatic(),                                      "   not static") +
            test(abstractWriteMethod.isAbstract(),                                     "   abstract") +
            test(!abstractWriteMethod.isGetter(),                                      "   not getter") +
            test(!abstractWriteMethod.isSetter(),                                      "   not setter") +
            test(closeMethod.getName() == "close",                             "MethodName: close") +
            test(closeMethod.getAttributes() == Attributes.INHERITED,          "   inherited attrib") +
            test(closeMethod.getAccessModifier() == AccessModifier.PROTECTED,  "   protected access modifier") +
            test(closeMethod.isInherited(),                                    "   inherited") +
            test(!closeMethod.isStatic(),                                      "   not static") +
            test(!closeMethod.isAbstract(),                                    "   not abstract") +
            test(!closeMethod.isGetter(),                                      "   not getter") +
            test(!closeMethod.isSetter(),                                      "   not setter")

}

function main(): int {
    let failures = 0
    failures += testMethodType()
    failures += testMethod()
    if (failures == 0) {
        console.println("PASSED: All tests run")
    } else {
        console.println("FAILED: All tests run")
    }
    return failures
}
