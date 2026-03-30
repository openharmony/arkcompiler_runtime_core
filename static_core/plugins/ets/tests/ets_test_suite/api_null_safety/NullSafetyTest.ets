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

namespace unsafe {

export native function leakString(): string

export native function leakClass(): Class

export native function leakLinker(): RuntimeLinker

export native function leakClassArray(): FixedArray<Class>

export native function leakByteArray(): ValueArray<byte>

}

function testClass() {
    arktest.expectThrow(() => { Class.from<string>().isSubtypeOf(unsafe.leakClass()) },
        (e: Error): boolean => e instanceof NullPointerError)
}

class MockHandler implements reflect.InvocationHandler {
    override get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        return undefined
    }

    override set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
    }

    override invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        return undefined
    }
}

function testProxy() {
    const handler = new MockHandler()
    arktest.expectThrow(() => { reflect.Proxy.create(unsafe.leakLinker(), [], handler) },
        (e: Error): boolean => e instanceof NullPointerError)
    arktest.expectThrow(() => { reflect.Proxy.create(getBootRuntimeLinker(), unsafe.leakClassArray(), handler) },
        (e: Error): boolean => e instanceof NullPointerError)
}

function testReflect() {
    arktest.expectThrow(() => { reflect.internals.createFixedArray(unsafe.leakClass(), 1) },
        (e: Error): boolean => e instanceof NullPointerError)
}

function testRuntimeLinker() {
    const linker = new AbcRuntimeLinker(undefined, [])

    arktest.expectThrow(() => { loadLibrary(unsafe.leakString()) },
        (e: Error): boolean => e instanceof NullPointerError)
    arktest.expectThrow(() => { loadLibraryWithPermissionCheck("test", unsafe.leakString()) },
        (e: Error): boolean => e instanceof NullPointerError)
    arktest.expectThrow(() => { loadLibraryWithPermissionCheck(unsafe.leakString(), "test") },
        (e: Error): boolean => e instanceof NullPointerError)

    arktest.expectThrow(() => { getBootRuntimeLinker().findLoadedClass(unsafe.leakString()) },
        (e: Error): boolean => e instanceof NullPointerError)
}

function main(): int {
    loadLibrary("unsafe_lib")

    let ts = new arktest.ArkTestsuite("Reflection API null safety test")

    ts.addTest("std.core.Class", testClass)
    ts.addTest("std.core.reflect.Proxy", testProxy)
    ts.addTest("std.core.reflect", testReflect)
    ts.addTest("std.core.RuntimeLinker", testRuntimeLinker)

    return ts.run();
}
