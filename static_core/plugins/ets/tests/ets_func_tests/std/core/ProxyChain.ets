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

interface Calculator {
    add(a: int, b: int): int
}

class CalcImpl implements Calculator {
    public add(a: int, b: int): int {
        // Force a GC inside the innermost call so the GC stack walker has to
        // traverse the surrounding chain of proxy-trampoline frames while scanning roots.
        GC.startGC(GC.Cause.YOUNG)
        return a + b
    }
}

class DelegatingHandler implements reflect.InvocationHandler {
    private next: Calculator
    static invocations: int = 0
    constructor(next: Calculator) { this.next = next }
    get(p: reflect.Proxy, m: reflect.InstanceMethod): Any { return 0 as Any }
    set(p: reflect.Proxy, m: reflect.InstanceMethod, v: Any): void { }
    invoke(p: reflect.Proxy, m: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        DelegatingHandler.invocations++
        return m.invoke(this.next as Object, args)
    }
}

function wrap(c: Calculator): Calculator {
    let linker = Class.of(c as Object).getLinker()
    let interfaces: FixedArray<Class> = [ Class.from<Calculator>() ]
    return reflect.Proxy.create(linker, interfaces, new DelegatingHandler(c)) as Calculator
}

function chain(depth: int): Calculator {
    let c: Calculator = new CalcImpl()
    for (let i = 0; i < depth; i++) {
        c = wrap(c)
    }
    return c
}

function testSingleProxy(): void {
    DelegatingHandler.invocations = 0
    let proxy = chain(1)
    arktest.assertEQ(proxy.add(1, 2), 3, "single proxy should forward add and return 3")
    arktest.assertEQ(DelegatingHandler.invocations, 1, "handler should be invoked exactly once")
}

function testProxyChainMinDepth(): void {
    DelegatingHandler.invocations = 0
    let proxy = chain(2)
    arktest.assertEQ(proxy.add(10, 20), 30, "2-deep proxy chain should return 30")
    arktest.assertEQ(DelegatingHandler.invocations, 2, "each of the 2 layers should delegate once")
}

function testDeepProxyChainWithGc(): void {
    DelegatingHandler.invocations = 0
    let proxy = chain(8)
    arktest.assertEQ(proxy.add(2, 3), 5, "8-deep proxy chain should return 5 even across GC")
    arktest.assertEQ(DelegatingHandler.invocations, 8, "all 8 layers should delegate")
}

function main(): int {
    const suite = new arktest.ArkTestsuite("reflect.Proxy chain with GC")
    suite.addTest("Single proxy forwards correctly", testSingleProxy)
    suite.addTest("Two-deep proxy chain (min depth for stack-walk path)", testProxyChainMinDepth)
    suite.addTest("Deep proxy chain survives GC during invocation", testDeepProxyChainWithGc)
    return suite.run()
}
