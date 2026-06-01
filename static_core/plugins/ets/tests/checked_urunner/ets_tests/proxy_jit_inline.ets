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

//! CHECKER      JIT must not inline a proxy method over a default interface method
//! RUN          entry: "proxy_jit_inline.ETSGLOBAL::main", \
//!              options: "--compiler-enable-jit=true --no-async-jit=true \
//!                 --compiler-regex='.*__noinline__callIntFoo' \
//!                 --compiler-hotness-threshold=2000 --compiler-profiling-threshold=0"
//! METHOD       "__noinline__callIntFoo"
//! PASS_AFTER   "Inline"
//! INST         /CallResolvedVirtual.*IntDefaultIface::foo/

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
        return INT_SENTINEL as Any
    }
}

function __noinline__callIntFoo(proxy: IntDefaultIface): int {
    return proxy.foo()
}

function createIntProxy(): IntDefaultIface {
    let target = new IntDefaultImpl()
    let linker = Class.of(target).getLinker()
    let interfaces: FixedArray<Class> = [ Class.from<IntDefaultIface>() ]
    return reflect.Proxy.create(linker, interfaces, new IntSentinelHandler()) as IntDefaultIface
}

const INT_PROXY: IntDefaultIface = createIntProxy()

function main(): int {
    let acc = 0
    for (let i = 0; i < 4000; ++i) {
        acc += __noinline__callIntFoo(INT_PROXY)
    }
    arktest.assertEQ(acc, INT_SENTINEL * 4000)
    return 0
}
