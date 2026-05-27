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

export interface I {
    checkPrimitives(b: boolean, by: byte, c: char, s: short, i: int, l: long, f: float, d: double): int
    checkReferences(s: string, i: int): string
    get primitiveProp(): int
    set primitiveProp(value: int)
    get referenceProp(): string
    set referenceProp(value: string)
}

native function doCheckPrimitives(b: boolean, by: byte, c: char, s: short, i: int, l: long, f: float, d: double): int
native function doCheckReferences(s: string, i: int): string
native function doCheckGetPrimitiveProp(): int
native function doCheckSetPrimitiveProp(value: int): void
native function doCheckGetReferenceProp(): string
native function doCheckSetReferenceProp(value: string): void

class ProxyHandler implements reflect.InvocationHandler {
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        if (method.getName() == '%%get-primitiveProp') {
            return doCheckGetPrimitiveProp()
        }
        return doCheckGetReferenceProp()
    }

    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
        if (method.getName() == '%%set-primitiveProp') {
            doCheckSetPrimitiveProp(value as int)
        } else {
            doCheckSetReferenceProp(value as string)
        }
    }

    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        if (method.getName() == 'checkPrimitives') {
            return doCheckPrimitives(
                args[0] as boolean,
                args[1] as byte,
                args[2] as char,
                args[3] as short,
                args[4] as int,
                args[5] as long,
                args[6] as float,
                args[7] as double
            )
        }
        return doCheckReferences(
            args[0] as string,
            args[1] as int
        )
    }
}

export function createProxy(): I {
    return reflect.Proxy.create(Class.current().getLinker(), [Class.from<I>()], new ProxyHandler) as I
}
