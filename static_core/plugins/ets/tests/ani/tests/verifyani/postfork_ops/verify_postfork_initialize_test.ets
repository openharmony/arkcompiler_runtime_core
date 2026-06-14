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

class PostforkManagedClass {
    public value: int = 17;

    public method(): int {
        return this.value;
    }
}

export native function postforkModuleNative(): int;

function postforkManagedValue(): int {
    return new PostforkManagedClass().method()
}

interface PostforkProxyIface {
    foo(): int
}

class PostforkProxyHandler implements reflect.InvocationHandler {
    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        return 42
    }

    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        return this.invoke(proxy, method, [])
    }

    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {}
}

function createPostforkReflectProxy(): PostforkProxyIface {
    let linker = Class.current().getLinker()
    let iface = Class.from<PostforkProxyIface>()
    let handler = new PostforkProxyHandler()
    return reflect.Proxy.create(linker, [iface], handler) as PostforkProxyIface
}

function callPostforkReflectProxy(): int {
    return createPostforkReflectProxy().foo()
}
