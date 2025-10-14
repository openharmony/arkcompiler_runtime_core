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

class SomeObject {
    constructor(other: int) {
        this.point = other
    }
    point: int = 0
}

interface AIface {
    message: string

    foo(): int
    boo(x: int, y: SomeObject): SomeObject
    goo(): double
}

class A implements AIface {
    message: string = "some default string"

    foo(): int {
        return 111
    }
    boo(x: int, y: SomeObject): SomeObject {
        return new SomeObject(x + y.point)
    }
    goo(): double {
        return 333.333
    }
}

interface BIface {
    message: string

    foo(): int
}

class AB implements AIface, BIface {
    message: string = "some default string"

    foo(): int {
        return 111
    }
    boo(x: int, y: SomeObject): SomeObject {
        return new SomeObject(x + y.point)
    }
    goo(): double {
        return 333.333
    }
}

class Base {}
class Derived extends Base {}

interface CIface {
    foo(b: Base): int
}

interface DIface {
    foo(d: Derived): int
}

class CD implements CIface, DIface {
    foo(b: Base) { return 1 }
}

enum Event {
    NONE,
    GETTER_CALLED,
    SETTER_CALLED,
    METHOD_CALLED,
}

let BAD_OWNER_ERROR = "method has bad owner class"

let LATEST_EVENT: Event = Event.NONE

class Handler implements reflect.InvocationHandler {
    instance: A | AB | CD
    ownerClass: Class | undefined

    constructor(other: A | AB | CD, ownerClass: Class | undefined) {
        this.instance = other
        this.ownerClass = ownerClass
    }

    setOwner(owner: Class) {
        this.ownerClass = owner
    }

    private checkMethod(method: reflect.InstanceMethod) {
        if (method.getOwner() != this.ownerClass) {
            throw new Error(BAD_OWNER_ERROR)
        }
        if (method == undefined) {
            throw new Error(`method is undefined`)
        }
    }

    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        LATEST_EVENT = Event.GETTER_CALLED
        this.checkMethod(method)
        return method.invoke(this.instance)
    }

    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
        LATEST_EVENT = Event.SETTER_CALLED
        this.checkMethod(method)
        method.invoke(this.instance, [value])
    }

    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        LATEST_EVENT = Event.METHOD_CALLED
        this.checkMethod(method)
        return method.invoke(this.instance, args)
    }
}

function testGetHandler(proxyHandler: reflect.InvocationHandler, proxy: reflect.Proxy, otherHandler: reflect.InvocationHandler) {
    arktest.assertEQ((proxy as reflect.Proxy).getHandler(), proxyHandler)
    arktest.assertNE((proxy as reflect.Proxy).getHandler(), otherHandler)
}

function testEmpty() {
    let stubObj = new A()
    let handler = new Handler(stubObj, undefined)
    let proxyInstance = reflect.Proxy.create(Class.of(stubObj).getLinker(), [], handler)

    testGetHandler(handler, proxyInstance, new Handler(stubObj, undefined))
}

function testInvokeAIface(proxyObj: AIface) {
    let fooRes = proxyObj.foo()
    arktest.assertTrue(fooRes instanceof int)
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertEQ(fooRes, 111)
    LATEST_EVENT = Event.NONE

    let booArg1 = 10
    let someObjectArg = 115
    let booArgs2 = new SomeObject(someObjectArg)
    let booRes = proxyObj.boo(booArg1, booArgs2)
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(booRes instanceof SomeObject)
    arktest.assertEQ(booRes.point, booArg1 + someObjectArg)
    LATEST_EVENT = Event.NONE

    let gooRes = proxyObj.goo()
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(gooRes instanceof double)
    arktest.assertEQ(gooRes, 333.333)
    LATEST_EVENT = Event.NONE
}

function testSetGetAIface(proxyObj: AIface) {
    let newMessage = "new message"
    proxyObj.message = newMessage
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    LATEST_EVENT = Event.NONE

    let getRes = proxyObj.message
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    arktest.assertEQ(getRes, newMessage)
    LATEST_EVENT = Event.NONE
}

function testAIface() {
    let obj = new A()
    let linker = Class.of(obj).getLinker()
    let aIfaceClass = (Type.from<AIface>() as InterfaceType).getClass()
    let interfaces: FixedArray<Class> = [ aIfaceClass ]
    let handler = new Handler(obj, aIfaceClass)
    let proxyInstance = reflect.Proxy.create(linker, interfaces, handler)
    let proxyAIface = proxyInstance as AIface

    testGetHandler(handler, proxyInstance, new Handler(new A, undefined))
    testInvokeAIface(proxyAIface)
    testSetGetAIface(proxyAIface)
}

function testInvokeBIface(proxyObj: BIface) {
    let fooRes = proxyObj.foo()
    arktest.assertTrue(fooRes instanceof int)
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertEQ(fooRes, 111)
    LATEST_EVENT = Event.NONE
}

function testSetGetBIface(proxyObj: BIface) {
    let newMessage = "new message"
    proxyObj.message = newMessage
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    LATEST_EVENT = Event.NONE

    let getRes = proxyObj.message
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    arktest.assertEQ(getRes, newMessage)
    LATEST_EVENT = Event.NONE
}

function testABProxyClass() {
    let obj = new AB()
    let linker = Class.of(obj).getLinker()
    let interfaces1: FixedArray<Class> = [ (Type.from<AIface>() as InterfaceType).getClass(),
                                           (Type.from<BIface>() as InterfaceType).getClass() ]
    let handler1 = new Handler(obj, (Type.from<AIface>() as InterfaceType).getClass())
    let proxyInstance1 = reflect.Proxy.create(linker, interfaces1, handler1)

    const errorcheck = (e: Error | Exception) => {
        return e.message === BAD_OWNER_ERROR
    }

    testGetHandler(handler1, proxyInstance1, new Handler(new AB, undefined))
    testInvokeAIface(proxyInstance1 as AIface)
    testSetGetAIface(proxyInstance1 as AIface)

    handler1.setOwner((Type.from<BIface>() as InterfaceType).getClass())
    arktest.expectThrow((): void => { testInvokeBIface(proxyInstance1 as BIface) }, errorcheck)
    arktest.expectThrow((): void => { testSetGetBIface(proxyInstance1 as BIface) }, errorcheck)

    let interfaces2: FixedArray<Class> = [ (Type.from<BIface>() as InterfaceType).getClass(),
                                           (Type.from<AIface>() as InterfaceType).getClass() ]
    let handler2 = new Handler(obj, (Type.from<BIface>() as InterfaceType).getClass())
    let proxyInstance2 = reflect.Proxy.create(linker, interfaces2, handler2)

    testGetHandler(handler2, proxyInstance2, new Handler(new AB, undefined))
    testInvokeBIface(proxyInstance2 as BIface)
    testSetGetBIface(proxyInstance2 as BIface)

    handler2.setOwner((Type.from<AIface>() as InterfaceType).getClass())
    arktest.expectThrow((): void => { testInvokeAIface(proxyInstance2 as AIface) }, errorcheck)
    arktest.expectThrow((): void => { testSetGetAIface(proxyInstance2 as AIface) }, errorcheck)
}

function testInvokeCIface(proxyObj: CIface) {
    let fooRes = proxyObj.foo(new Base())
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof int)
    arktest.assertEQ(fooRes, 1)
    LATEST_EVENT = Event.NONE
}

function testInvokeDIface(proxyObj: DIface) {
    let fooRes = proxyObj.foo(new Derived())
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof int)
    arktest.assertEQ(fooRes, 1)
    LATEST_EVENT = Event.NONE
}

function testOverloads() {
    let obj = new CD()
    let linker = Class.of(obj).getLinker()
    let interfaces1: FixedArray<Class> = [ (Type.from<CIface>() as InterfaceType).getClass(),
                                           (Type.from<DIface>() as InterfaceType).getClass() ]
    let handler1 = new Handler(obj, (Type.from<CIface>() as InterfaceType).getClass())
    let proxyInstance1 = reflect.Proxy.create(linker, interfaces1, handler1)

    testInvokeCIface(proxyInstance1 as CIface)
    testInvokeDIface(proxyInstance1 as DIface) // owner type still CIface

    let interfaces2: FixedArray<Class> = [ (Type.from<DIface>() as InterfaceType).getClass(),
                                           (Type.from<CIface>() as InterfaceType).getClass() ]
    let handler2 = new Handler(obj, (Type.from<DIface>() as InterfaceType).getClass())
    let proxyInstance2 = reflect.Proxy.create(linker, interfaces2, handler2)

    testInvokeDIface(proxyInstance2 as DIface)

    const errorcheck = (e: Error | Exception) => {
        return e instanceof LinkerAbstractMethodError
    }

    handler2.setOwner((Type.from<CIface>() as InterfaceType).getClass())
    arktest.expectThrow((): void => { testInvokeCIface(proxyInstance2 as CIface) }, errorcheck)
}

function main() {
    const suite = new arktest.ArkTestsuite('Proxy test')
    suite.addTest('Test proxy for emptry interface', testEmpty)
    suite.addTest('Test proxy for single interface', testAIface)
    // Enable after 29358 is fixed.
    // suite.addTest('Test proxy for two interface', testABProxyClass)
    // suite.addTest('Test proxy for two interface with overloads and override-compatible signatures', testOverloads)
    return suite.run()
}
