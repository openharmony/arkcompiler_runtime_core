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

export class Base {}
export class Derived extends Base {}

export interface MyInterface {
    foo(value: int): int
    foo(value: Derived): double
}

export class MyInterfaceImpl implements MyInterface {
    foo(value: int): int { return 1 }
    foo(value: Base): double { return 12.12 }
}

export let instance = new MyInterfaceImpl

export function testMyInterface(proxyObj: reflect.Proxy) {
    testMyInterfaceImpl(proxyObj as MyInterface)
}

function testMyInterfaceImpl(proxyObj: MyInterface){
    arktest.assertEQ(proxyObj.foo(10), 1)

    // MyInterface2 has more subtype signature which will be taken by proxy.
    const errorcheck = (e: Error) => {
        return (e instanceof TypeError)
            && e.message === "Object type [myinterface.MyInterfaceImpl] is not compatible with method owner type [myinterface2.MyInterface2]"
    }
    arktest.expectThrow(() => { proxyObj.foo(new Derived) }, errorcheck)
}
