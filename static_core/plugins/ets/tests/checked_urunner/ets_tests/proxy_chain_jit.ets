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

//! CHECKER      JIT must not inline a proxy method at the head of a delegating chain
//! SKIP_IF      @architecture == "arm32"
//! RUN          entry: "proxy_chain_jit.ETSGLOBAL::main", \
//!              options: "--compiler-enable-jit=true --no-async-jit=true \
//!                 --compiler-regex='.*__noinline__callAdd' \
//!                 --compiler-hotness-threshold=2000 --compiler-profiling-threshold=0"
//! METHOD       "__noinline__callAdd"
//! PASS_AFTER   "Inline"
//! INST         /CallResolvedVirtual.*Calculator::add/

const CHAIN_DEPTH: int = 5
const WARMUP: int = 3000

interface Calculator {
    add(a: int, b: int): int { return a + b }
}

class CalcImpl implements Calculator {
    static calls: int = 0
    add(a: int, b: int): int {
        CalcImpl.calls++
        if (CalcImpl.calls % 500 == 0) {
            GC.startGC(GC.Cause.YOUNG)
        }
        return a + b
    }
}

class DelegatingHandler implements reflect.InvocationHandler {
    private next: Calculator
    static invocations: int = 0
    constructor(next: Calculator) { this.next = next }
    get(p: reflect.Proxy, m: reflect.InstanceMethod): Any { return 0 as Any }
    set(p: reflect.Proxy, m: reflect.InstanceMethod, v: Any): void {}
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

function __noinline__callAdd(proxy: Calculator): int {
    return proxy.add(2, 3)
}

const PROXY: Calculator = chain(CHAIN_DEPTH)

function main(): int {
    let acc = 0
    for (let i = 0; i < WARMUP; i++) {
        DelegatingHandler.invocations = 0
        acc += __noinline__callAdd(PROXY)
        arktest.assertEQ(DelegatingHandler.invocations, CHAIN_DEPTH,
                         "every layer of the chain must delegate")
    }
    arktest.assertEQ(acc, 5 * WARMUP, "chain must return 5 on every call, even across GC")
    return 0
}
