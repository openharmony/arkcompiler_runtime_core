/**
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

import { attach, fns } from './utils'

interface ProxyOps {
    compute(lhs: int, rhs: int): int
}

class ProxyTarget implements ProxyOps {
    compute(lhs: int, rhs: int): int {
        return lhs + rhs
    }
}

class ProxyHandler implements reflect.InvocationHandler {
    private target: ProxyOps

    constructor(target: ProxyOps) {
        this.target = target
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

function createProxy(): ProxyOps {
    const target = new ProxyTarget()
    const interfaces: FixedArray<Class> = [Class.from<ProxyOps>()]
    const proxy = reflect.Proxy.create(Class.of(target).getLinker(), interfaces, new ProxyHandler(target))
    arktest.assertTrue(reflect.Proxy.isProxyClass(Class.of(proxy)))
    return proxy as ProxyOps
}

function main() {
    loadLibrary('ets_debug_session_attach_lib')

    const proxyBeforeAttach = createProxy()
    arktest.assertEQ(proxyBeforeAttach.compute(1, 2), 3)
    arktest.assertEQ(0, attach.countEventsForMethod('compute'))

    arktest.assertEQ(attach.attachDebugSession(), attach.DebugSessionAttachErrorCode.OK)

    arktest.assertEQ(proxyBeforeAttach.compute(1, 2), 3)
    const computeEvents = attach.countEventsForMethod('compute')
    arktest.assertLT(0, computeEvents)
    const proxyAfterAttach = createProxy()
    arktest.assertEQ(proxyAfterAttach.compute(1, 2), 3)
    arktest.assertLT(computeEvents, attach.countEventsForMethod('compute'))

    fns.sum(1, 2)
    arktest.assertLT(0, attach.countEventsForMethod('sum'))
}
