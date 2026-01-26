/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

function getAdditionalFilesList(): string[] {
    let procManager = new StdProcess.ProcessManager();
    const abcFiles = procManager.getEnvironmentVar("ADDITIONAL_ABC_FILES");
    arktest.assertLT(0, abcFiles.length, "this test must have additional abc files")
    return abcFiles.split(":");
}

class Handler<T extends Object> implements reflect.InvocationHandler {
    target: T

    constructor(other: T) {
        this.target = other
    }

    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        return method.invoke(this.target)
    }

    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
        method.invoke(this.target, [value])
    }

    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        return method.invoke(this.target, args)
    }
}

function getTargetObject(linker: RuntimeLinker): Object {
    let global: Class | undefined = linker.loadClass("myinterface.ETSGLOBAL", true)
    arktest.assertNE(global, undefined)

    let targetAsStaticField: reflect.StaticField | undefined = global!.getStaticField("instance")
    arktest.assertNE(targetAsStaticField, undefined)

    return targetAsStaticField!.getValue() as Object
}

function invokeTestMyInterfaceImpl(linker: RuntimeLinker, proxyObj: reflect.Proxy) {
    let global: Class | undefined = linker.loadClass("myinterface.ETSGLOBAL", true)
    arktest.assertNE(global, undefined)

    let staticMethod: reflect.StaticMethod | undefined = global!.getStaticMethod("testMyInterface")
    arktest.assertNE(staticMethod, undefined)

    staticMethod!.invoke( [ proxyObj ])
}

function test1(abcFiles: Array<string>) {
    let linker = new AbcRuntimeLinker(undefined, [ abcFiles[0] ])
    let linker2 = new AbcRuntimeLinker(undefined, [ abcFiles[1] ])
    arktest.assertNE(linker, Class.current().getLinker())
    arktest.assertNE(linker, Class.current().getLinker())

    let myInterfaceCls: Class | undefined = linker.loadClass("myinterface.MyInterface", true)
    arktest.assertNE(myInterfaceCls, undefined)

    let myInterface2Cls: Class | undefined = linker2.loadClass("myinterface2.MyInterface2", true)
    arktest.assertNE(myInterface2Cls, undefined)

    let interfaces: FixedArray<Class> = [ myInterfaceCls!, myInterface2Cls! ]
    let target: Object = getTargetObject(linker)
    let handler = new Handler<Object>(target)

    let proxyInstance = reflect.Proxy.create(Class.current().getLinker(), interfaces, handler)
    invokeTestMyInterfaceImpl(linker, proxyInstance)
}

function main() {
    let abcFiles: Array<string> = getAdditionalFilesList()
    arktest.assertEQ(abcFiles.length, 2)

    test1(abcFiles)
}
