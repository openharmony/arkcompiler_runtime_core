/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

package reflect_proxy.test;

export class SomeObject {
    constructor(other: int) {
        this.point = other;
    }
    public point: int = 0;
}

export interface AIface {
    messageA: string;

    foo(): int;
    boo(x: int, y: SomeObject): SomeObject
    fooCalled(): boolean;
    booCalled(): boolean;
}

export interface BIface {
    messageB: string

    goo(): int;
    gooCalled(): boolean;
}

export class A implements AIface {
    messageA: string = 'some default string A';
    fooIsCalled: boolean = false;
    booIsCalled: boolean = false;

    foo(): int {
        this.fooIsCalled = true;
        return 111
    }
    boo(x: int, y: SomeObject): SomeObject {
        this.booIsCalled = true;
        return new SomeObject(x + y.point);
    }
    fooCalled(): boolean {
        return this.fooIsCalled;
    }
    booCalled(): boolean {
        return this.booIsCalled;
    }
}

export class AB implements AIface, BIface {
    messageA: string = 'some default string A';
    messageB: string = 'some default string B';
    fooIsCalled: boolean = false;
    booIsCalled: boolean = false;
    gooIsCalled: boolean = false;

    foo(): int {
        this.fooIsCalled = true;
        return 111;
    }
    boo(x: int, y: SomeObject): SomeObject {
        this.booIsCalled = true;
        return new SomeObject(x + y.point);
    }
    goo(): int {
        this.gooIsCalled = true;
        return 123;
    }
    fooCalled(): boolean {
        return this.fooIsCalled;
    }
    booCalled(): boolean {
        return this.booIsCalled;
    }
    gooCalled(): boolean {
        return this.gooIsCalled;
    }
}

class BasicHandler implements reflect.InvocationHandler {
    instance: AIface;
    constructor(other: AIface) {
        this.instance = other;
    }
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        return method.invoke(this.instance);
    }

    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
        method.invoke(this.instance, [value]);
    }

    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        return method.invoke(this.instance, args);
    }
}

class ABFaceHandler implements reflect.InvocationHandler {
    instance: AIface | BIface;
    constructor(other: AIface | BIface) {
        this.instance = other;
    }
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        return method.invoke(this.instance);
    }

    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
        method.invoke(this.instance, [value]);
    }

    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        return method.invoke(this.instance, args);
    }
}

export let objA = new A();
export let objAB = new AB();

export function getProxyA(): AIface {
    let linker = Class.of(objA).getLinker();
    let interfaces: FixedArray<Class> = [ Class.from<AIface>() ];
    return reflect.Proxy.create(linker, interfaces, new BasicHandler(objA)) as AIface;
}

export function getProxyAB(): AIface | BIface {
    let linker = Class.of(objAB).getLinker();
    let interfaces: FixedArray<Class> = [ Class.from<AIface>(), Class.from<BIface>() ];
    return reflect.Proxy.create(linker, interfaces, new ABFaceHandler(objAB)) as AIface | BIface;
}

export function testProxyA(a: AIface, message: string) {
    arktest.assertEQ(a.messageA, message);
    arktest.assertEQ(a.fooCalled(), true);
    arktest.assertEQ(a.booCalled(), true);
}

export function testProxyAB(ab: AIface | BIface, messageA: string, messageB: string) {
    testProxyA(ab as AIface, messageA);
    let b: BIface = ab as BIface;
    arktest.assertEQ(b.messageB, messageB);
    arktest.assertEQ(b.gooCalled(), true);
}