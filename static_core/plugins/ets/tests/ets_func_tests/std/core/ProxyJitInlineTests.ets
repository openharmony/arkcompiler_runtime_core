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

enum Event {
    NONE,
    METHOD_CALLED,
}

let LATEST_EVENT: Event = Event.NONE

const INT_DEFAULT_BODY: int = 1
const INT_SENTINEL: int = 777

interface IntDefaultIface {
    foo(): int { return INT_DEFAULT_BODY }
}

class IntDefaultImpl implements IntDefaultIface {}

class IntSentinelHandler implements reflect.InvocationHandler {
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any { return 0 as Any }
    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {}
    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        LATEST_EVENT = Event.METHOD_CALLED
        return INT_SENTINEL as Any
    }
}

function __noinline__callIntFoo(proxy: IntDefaultIface): int {
    return proxy.foo()
}

const DOUBLE_DEFAULT_BODY: double = 1.0
const DOUBLE_SENTINEL: double = 4.2

interface DoubleDefaultIface {
    foo(): double { return DOUBLE_DEFAULT_BODY }
}

class DoubleDefaultImpl implements DoubleDefaultIface {}

class DoubleSentinelHandler implements reflect.InvocationHandler {
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any { return 0 as Any }
    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {}
    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        LATEST_EVENT = Event.METHOD_CALLED
        return DOUBLE_SENTINEL as Any
    }
}

function __noinline__callDoubleFoo(proxy: DoubleDefaultIface): double {
    return proxy.foo()
}

const STRING_DEFAULT_BODY: string = "default"
const STRING_SENTINEL: string = "handler"

interface StringDefaultIface {
    foo(): string { return STRING_DEFAULT_BODY }
}

class StringDefaultImpl implements StringDefaultIface {}

class StringSentinelHandler implements reflect.InvocationHandler {
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any { return 0 as Any }
    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {}
    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        LATEST_EVENT = Event.METHOD_CALLED
        return STRING_SENTINEL as Any
    }
}

function __noinline__callStringFoo(proxy: StringDefaultIface): string {
    return proxy.foo()
}

function createIntProxy(): IntDefaultIface {
    let target = new IntDefaultImpl()
    let linker = Class.of(target).getLinker()
    let interfaces: FixedArray<Class> = [ Class.from<IntDefaultIface>() ]
    return reflect.Proxy.create(linker, interfaces, new IntSentinelHandler()) as IntDefaultIface
}

function createDoubleProxy(): DoubleDefaultIface {
    let target = new DoubleDefaultImpl()
    let linker = Class.of(target).getLinker()
    let interfaces: FixedArray<Class> = [ Class.from<DoubleDefaultIface>() ]
    return reflect.Proxy.create(linker, interfaces, new DoubleSentinelHandler()) as DoubleDefaultIface
}

function createStringProxy(): StringDefaultIface {
    let target = new StringDefaultImpl()
    let linker = Class.of(target).getLinker()
    let interfaces: FixedArray<Class> = [ Class.from<StringDefaultIface>() ]
    return reflect.Proxy.create(linker, interfaces, new StringSentinelHandler()) as StringDefaultIface
}

const INT_PROXY: IntDefaultIface = createIntProxy()
const DOUBLE_PROXY: DoubleDefaultIface = createDoubleProxy()
const STRING_PROXY: StringDefaultIface = createStringProxy()

function testIntDefaultMethodNotInlined() {
    LATEST_EVENT = Event.NONE
    arktest.assertEQ(__noinline__callIntFoo(INT_PROXY), INT_SENTINEL)
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
}

function testDoubleDefaultMethodNotInlined() {
    LATEST_EVENT = Event.NONE
    arktest.assertEQ(__noinline__callDoubleFoo(DOUBLE_PROXY), DOUBLE_SENTINEL)
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
}

function testStringDefaultMethodNotInlined() {
    LATEST_EVENT = Event.NONE
    arktest.assertEQ(__noinline__callStringFoo(STRING_PROXY), STRING_SENTINEL)
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Proxy JIT inline regression test')

    suite.addTest('Default int method on hot proxy call site must be not inlined', testIntDefaultMethodNotInlined)
    suite.addTest('Default double method on hot proxy call site must be not inlined', testDoubleDefaultMethodNotInlined)
    suite.addTest('Default string method on hot proxy call site must be not inlined', testStringDefaultMethodNotInlined)

    return suite.run()
}
