/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

class Base {
    private _id: number = 0

    getId(): number {
        return this._id
    }
}

class ObjWithProps extends Base {
    private _name: string = ""

    get name(): string {
        return this._name
    }

    set name(name: string) {
        this._name = name
    }

    isNameSet(): boolean {
        return this._name != ""
    }

    add(name: string): void {
        this._name += "," + name
    }

    add(name1: string, name2: string): void {
        this._name += "," + name1 + "," + name2
    }

    toString(): string {
        return `ObjWithProps: { _name: '${this.name}' }`
    }
}

class ObjProxyHandler extends DefaultProxyHandler<ObjWithProps> {
    getTrapped: boolean = false
    setTrapped: boolean = false
    getIdTrapped: boolean = false

    get(target: ObjWithProps, name: string): NullishType {
        this.getTrapped = true

        const value = super.get(target, name)
        if (name == "name") {
            return value + "1"
        } else {
            return value
        }
    }

    set(target: ObjWithProps, name: string, value: NullishType): boolean {
        this.setTrapped = true
        return super.set(target, name, value)
    }

    invoke(target: ObjWithProps, method: Method, args: NullishType[]): NullishType {
        const methodName = method.getName()
        if (methodName == "isNameSet") {
            return false
        } else if (methodName == "getId") {
            this.getIdTrapped = true
        }

        return super.invoke(target, method, args)
    }
}

function verifyObjectProxyOps(obj: ObjWithProps, objProxyHandler: ObjProxyHandler): void {
    assertFalse(objProxyHandler.getTrapped)
    assertFalse(objProxyHandler.setTrapped)

    const objProxy = Proxy.create(obj, objProxyHandler)

    objProxy.name = "foo"
    assertTrue(objProxyHandler.setTrapped)

    const objName = objProxy.name
    assertTrue(objProxyHandler.getTrapped)

    assertEQ(objName,  "foo1")

    assertTrue(obj.isNameSet())
    assertFalse(objProxy.isNameSet())

    objProxy.add("bar")
    assertEQ(objProxy.name, "foo,bar1")

    objProxy.add("baz", "goo")
    assertEQ(objProxy.name, "foo,bar,baz,goo1")

    const id = objProxy.getId()
    assertTrue(objProxyHandler.getIdTrapped)

    assertEQ(Proxy.tryGetTarget(objProxy), obj)
    assertEQ(Proxy.tryGetHandler(objProxy), objProxyHandler)
}

function testObjectProxyOps(): void {
    verifyObjectProxyOps(new ObjWithProps(), new ObjProxyHandler())
}


class Handler extends DefaultProxyHandler<Map<string, string>> {}
function testStdLibClassesProxying(): void {
    const map = new Map<string, string>()

    const mapProxy = Proxy.create(map, new Handler())

    assertTrue(map === mapProxy)
}

function main(): int {
    const suite = new ArkTestsuite("Proxy API")

    suite.addTest("Object proxy operations", testObjectProxyOps)
    suite.addTest("StdLib classes proxying", testStdLibClassesProxying)

    return suite.run()
}
