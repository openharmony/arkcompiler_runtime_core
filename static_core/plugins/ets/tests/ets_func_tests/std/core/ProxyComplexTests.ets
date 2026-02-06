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

class Base {
    public value: double = 0
    constructor(v: double = 0) { this.value = v }
}
class Derived extends Base {
    constructor(v: double = 0) { super(v) }
}


interface A1 {
    foo(x: char): double { return -1.1 }
}

interface A2 {
    foo(x: number): double
}

interface A extends A1, A2 {
    foo(x: Derived): double { return 1.0 }
}

class AImpl implements A {
    override foo(x: Base): double { return 1.2 }
    override foo(x: number): double { return x }
}

class AImplChild extends AImpl {
    override foo(x: Object): double { return 42 }
}


interface B1 {
    foo(x: Base): double { return 1.1 }
}

class B1Impl implements B1 {
    foo(x: Base): double { return 2.2 } // overrides B1.foo
}

interface B2 {
    foo(x: Derived): double
}

class B2Impl extends B1Impl implements B2 {
    foo(x: Base): double { return 3.3 } // overrides B2.foo, B1Impl.foo
}


interface C1 {
    foo(x: string): double { return 1.1 }
}

class C1Impl implements C1 {
    foo(x: string): double { return 2.2 }
}

interface C2 {
    foo(x: int): double
}

class C2Impl extends C1Impl implements C2 {
    foo(x: int): double { return 3.3 }
}


interface D1 {
    foo(x: Derived): double

    boo(x: Derived): double { return 27.1 }

    goo(x: Derived): Object { return new Base(1) }
}

interface D2 extends D1 {
    foo(x: Base): double { return -511 }

    boo(x: Base, y: double): double { return y }

    goo(x: Derived): Base { return new Base(2) }
}

class DImpl implements D2 {
    goo(x: Derived): Derived { return new Derived(3) }
}


interface E1 {
    foo(x: Derived): double { return 111.111 }
    foo(x: double): double { return 111.222 }
}

interface E2 extends E1 {
    foo(x: Base): double
}

interface E3 extends E2, E5 {
    foo(x: Base): double { return 333.111 } // overrides E1.foo, implements E2.foo
}

interface E4 extends E3 {
    foo(x: int): double { return 444.111 }
    foo(x: char): double
}

interface E5 extends E2 {
    foo(x: Base): double { return 555.111 } // overrides E1.foo, implements E2.foo
}

interface E6 extends E5 {
    foo(x: Derived): double
    foo(x: Base): double
}

class EImpl implements E4, E6 {
    foo(x: Object): double { return 777.111 }
    foo(x: char): double { return 777.222 }
}


interface F1 {
    i: Int
    foo(x: Derived): double
}

interface F2 extends F1 {
    boo(x: Derived): double { return 27.1 }
}

interface F3 extends F2 {
    goo(x: Derived): Object { return new Base(1) }
}

interface F4 extends F3 {
    foo(x: Base): double { return -511 }
}

interface F5 extends F4 {
    boo(x: Base, y: double): double { return y }
}

interface F6 extends F5 {
    goo(x: Derived): Base { return new Base(2) }
}

class FImpl implements F6 {
    i: Int = 0
    goo(x: Derived): Derived { return new Derived(3) }
}


interface I1 {
    foo(): double { return 1 }
}

interface I2 extends I1 {
    foo(a: Int): double { return 2 }
}

interface I3 extends I1  {
    i: Int
    foo(a: Int, b: Int): double { return 3 }
}

interface I4 extends I1  {
    i: Int
    foo(a: Int, b: Int, c: Int): double { return 4 }
}

interface I5 extends I1  {
    i: Int
    foo(a: Int, b: Int, c: Int, d: Int): double { return 5 }
}

interface I6 extends I1  {
    foo(a: Int, b: Int, c: Int, d: Int, e: Int): double { return 6 }
}

interface I7 extends I2, I3, I4, I5, I6 {}

class IImpl1 implements I7 {
    i: Int
}

class IImpl2 implements I7 {
    i: Int
    override foo(a: Int): double { return 21 }
    override foo(a: Int, b: Int, c: Int): double { return 41 }
}


interface G1<T> {
    foo(x: Derived, arr: readonly T[]): double
}

interface G2 extends G1<number> {
    foo(a: Int): double {return 2}
}

interface G3<T> {
    foo(x: Derived, arr: readonly T[]): double { return 22 }
}

interface G4 extends G1<number>, G3<number> {
    foo(): double { return 4}
    foo(a: Int, b: Int): double { return 5}
}

interface G5 extends G2, G3<number>, G4 {
    foo(a: Int, b: Int, c: Int): double { return 6 }
}

class GImpl implements G5 {}


interface H1 {
    foo(a: Int): double
    foo(a: Int, b:Int): double
    foo(a: Int, b:Int, c:Int): double
}

interface H2 extends H1 {
    foo(a: Int): double { return 1 }
}

interface H3 extends H1  {
    foo(a: Int, b: Int): double { return 2 }
}

interface H4 extends H1  {
    foo(a: Int, b: Int, c: Int): double { return 3 }
}

interface H5 extends H4  {
    foo(a: Derived): double { return 4 }
}

interface H6 extends H2, H3  {
    foo(a: Derived, b: Int): double { return 5 }
}

interface H7 extends H5, H6, H2  {}

class HImpl implements H7 {}


interface J1 {
    foo(a: Int): double
    foo(a: Int, b: Number): double
}

interface J2 extends J1 {
    foo(a: Int): double { return 2 }
}

interface J3 extends J2 {
    foo(a: Int, b: Int, c:Int): double { return 3 }
}

interface J4 extends J1 {
    foo(a: Int, b: Number): double { return 4 }
}

interface J5 extends J4 {
    foo(a: Derived): double { return 5 }
}

interface J6 extends J3, J5 {}

class JImpl implements J6 {}


interface K1<T> {
    a: Int
    c: T
    foo(a: T): Int
}

interface K2 extends K1<Int> {
    c: Int
    foo(a: Int): Int { return 1 }
}

interface K3<T> {
    foo(a: T): Int { return 2 }
}

interface K4 extends K2, K3<Base> {}

class KImpl implements K4 {
    a: Int
    c: Int
}


interface L1 {
    a?: Int
    foo(a: Int): Int
}

interface L2 {
    foo(a: Double): Int
}

interface L3 {
    foo(a: String): Int
}

interface L4 extends L1 {
    foo(a: String): Int { return 1 }
}

interface L5 extends L1, L2, L3 {
    foo(a: Int): Int { return 2 }
    foo(a: Double): Int { return 3 }
}

interface L6 extends L4, L5 {}

class LImpl implements L6 {
    a?: Int
    override foo(a: String): Int {return 4}
}

class LImpl2 implements L4 {
    a?: Int
    foo(a: Int): Int { return 6}
}

class LImpl3 implements L5 {
    a?: Int
    foo(a: String): Int { return 5}
}


interface M1 {
    foo(a: Int): Int
    foo(a: Double): Int
}

interface M2 {
    foo(a: String): Int
    foo(a: Derived): Int
}

interface M3 extends M1 {
    foo(a: String): Int { return 1 }
}

interface M4 extends M1 {
    foo(a: Derived): Int { return 2 }
}

interface M5 {
    foo(a: Int): Int { return 3 }
}

interface M6 {
    foo(a: Double): Int { return 4 }
}

interface M7 extends M3, M4, M5, M6 {}

class MImpl implements M7 {}


interface O1 {
    foo(a: Int): Int
}

interface O2 {
    bar(a: Int): Int
}

interface O3 extends O1, O2 {
    foo(a: Int): Int { return 1 }
    bar(a: Int): Int { return 2 }
}

interface O4 extends O3 {}

interface O5 extends O3 {}

class OImpl1 implements O4 {
    override foo(a: Int): Int { return 3 }
}

class OImpl2 implements O5 {
    override bar(a: Int): Int { return 4 }
}


interface P1 {
    a?: Int
    foo(a: Int): Int
}

interface P2 {
    b?: Int
    foo(a: Int, b: Int): Int { return 1 }
}

interface P3 {
    c: Int
    foo(a: Int, b: Int, c: Base): Int
}

interface P4 extends P1, P2 {
    foo(a: Int): Int { return 2 }
    foo(a: Int, b: Int, c: Base): Int { return 6 }
}

interface P5 extends P3, P2 {
    foo(a: Int, b: Int): Int { return 3 }
}

interface P6 extends P5 {
    foo(a: Int, b: Int, c: Base): Int { return 5 }
}

class PImpl1 implements P4 {
    a?: Int
    b?: Int
}

class PImpl2 implements P6 {
    b?: Int
    c: Int
    override foo(a: Int, b: Int): Int { return 4 }
}


interface R1 {
    foo(): Int
    foo(a: Int): Int
    bar(): Int
}

interface R2 extends R1 {
    foo(): Int { return 1 }
}

interface R3 extends R1 {
    bar(): Int { return 2 }
}

interface R4 extends R2 {
    foo(a: Int = 1): Int { return 3 }
}

interface R5 extends R3 {
    buz(a: Int = 1): Int { return 4 }
}

interface R6 {
    a?: Int
}

interface R7<T> extends R6 {
    foo(a: T, b: Base): Int { return 5 }
}

interface R8 extends R7<Double> {
    foo(a: Base): Double { return 6.0 }
}

interface R9<T> extends R5, R8 {
    b?: Number
}

interface R10 extends R4, R6, R9<Double> {
    bar(a: Int, b:Int = 10): Int { return 7 }
}

interface R11 extends R10 {}

interface R12 extends R10 {}

class RImpl1 implements R11 {
    a?: Int
    b?: Number
}

class RImpl2 implements R12{
    a?: Int
    b?: Number
}


interface S1 {
    a: Int
}

interface S2 extends S1 {}

interface S3 extends S1 {}

class SImpl1 implements S2 {
    a: Int
}

class SImpl2 implements S3 {
    a: Int
}


interface N1 {
    foo(x: int): double { return 1.1 }
    a: Int
}

interface N2 extends N1 {
    foo(x: int): double { return 2.2 }
    b?: Int
}

interface N3 extends N1 {
    boo(x: Derived): double { return 3.0 }
}

class NTarget implements N2, N3 {
    a: Int = 10
    b?: Int = 20

    override foo(x: int): double { return 9.9 }
    override boo(x: Derived): double { return 30.0 }
}


interface Q1 {
    foo(x: Base): double { return 1.1 }
    a: Int
}

interface Q2 extends Q1 {
    goo(x: Derived): double { return 2.0 }
    b?: Int
}

interface Q3 extends Q1 {
    zoo(): string { return "Q3" }
}

class QTarget implements Q2, Q3 {
    a: Int = 10
    b?: Int = 20

    override foo(x: Base): double { return 9.9 }
    override goo(x: Derived): double { return 20.0 }
    override zoo(): string { return "QTarget" }
}


interface T1 {
    foo(x: int): double
    a: Int
}

interface T2 extends T1 {
    foo(x: int): double { return 2.2 }
    b?: Int
}

interface T3 {
    foo(x: Derived): double
    boo(): int { return 3 }
}

class TTarget implements T2, T3 {
    a: Int = 10
    b?: Int = 20

    override foo(x: int): double { return 9.9 }
    override foo(x: Derived): double { return 8.8 }
    override boo(): int { return 30 }
}


enum Event {
    NONE,
    GETTER_CALLED,
    SETTER_CALLED,
    METHOD_CALLED,
}

let LATEST_EVENT: Event = Event.NONE

class Handler<T extends Object> implements reflect.InvocationHandler {
    target: T

    constructor(other: T) {
        this.target = other
    }

    setTarget(target: T) {
        this.target = target
    }

    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        LATEST_EVENT = Event.GETTER_CALLED
        return method.invoke(this.target)
    }

    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
        LATEST_EVENT = Event.SETTER_CALLED
        method.invoke(this.target, [value])
    }

    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        LATEST_EVENT = Event.METHOD_CALLED
        return method.invoke(this.target, args)
    }
}

class DualTargetsHandler<T1 extends Object, T2 extends Object> implements reflect.InvocationHandler {
    private target1: T1
    private target2: T2

    private targetChooser: (m: reflect.InstanceMethod) => Object | undefined = ()=>{ return undefined }

    private chooseTarget(m: reflect.InstanceMethod): Object {
        return this.targetChooser(m)!
    }

    public createTargetChooser(cls1: Class, cls2: Class) {
        this.targetChooser = (method: reflect.InstanceMethod) => {
            let owner = method.getOwner()
            if (cls1.isSubtypeOf(owner)) {
                return this.target1
            } else if (cls2.isSubtypeOf(owner)) {
                return this.target2
            }
            throw new Error(`Method '${method.getName()}' has unexpected owner class '${method.getOwner()!.getName()}', `,
                            `expected '${cls1.getName()}' or '${cls2.getName()}'`)

        }
    }

    constructor(t1: T1, t2: T2) {
        this.target1 = t1
        this.target2 = t2
    }

    constructor(t1: T1, t2: T2, c1: Class, c2: Class) {
        this.target1 = t1
        this.target2 = t2
        this.createTargetChooser(c1, c2)
    }

    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        LATEST_EVENT = Event.GETTER_CALLED
        return method.invoke(this.chooseTarget(method))
    }

    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
        LATEST_EVENT = Event.SETTER_CALLED
        method.invoke(this.chooseTarget(method), [value])
    }

    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        LATEST_EVENT = Event.METHOD_CALLED
        return method.invoke(this.chooseTarget(method), args)
    }
}

function createProxy(interfaces: FixedArray<Class>, handler: reflect.InvocationHandler, linker: RuntimeLinker) {
    let proxyInstance = reflect.Proxy.create(linker, interfaces, handler)
    arktest.assertTrue(reflect.Proxy.isProxyClass(Class.of(proxyInstance)))
    arktest.assertEQ(proxyInstance.getHandler(), handler)
    return proxyInstance
}

function eventToString(event: Event): string {
    if (event == Event.SETTER_CALLED) {
        return "SETTER_CALLED"
    }
    if (event == Event.GETTER_CALLED) {
        return "GETTER_CALLED"
    }
    if (event == Event.METHOD_CALLED) {
        return "METHOD_CALLED"
    }
    if (event == Event.NONE) {
        return "NONE"
    }
    throw new Error("unreachable")
}

function testIsLastEvent(event: Event) {
    arktest.assertEQ(LATEST_EVENT, event, `Expected latest event ${eventToString(event)}, but got ${eventToString(LATEST_EVENT)}`)
    LATEST_EVENT = Event.NONE
}

function testIsLastEventSetter() {
    testIsLastEvent(Event.SETTER_CALLED)
}

function testIsLastEventGetter() {
    testIsLastEvent(Event.GETTER_CALLED)
}

function testIsLastEventMethod() {
    testIsLastEvent(Event.METHOD_CALLED)
}

function testA1IfaceAImpl(proxyObj: A1) {
    arktest.assertEQ(proxyObj.foo(c'1'), -1.1)
    testIsLastEventMethod()
}

function testA2IfaceAImpl(proxyObj: A2) {
    arktest.assertEQ(proxyObj.foo(100), 100)
    testIsLastEventMethod()
}

function testAIfaceAImpl(proxyObj: A, val1: double, val2: double, val3: double) {
    arktest.assertEQ(proxyObj.foo(new Derived), val1)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(val2), val2)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(c'1'), val3)
    testIsLastEventMethod()
}

function singleInterfaceTest1() {
    let target = new AImpl
    let proxyObj = createProxy([ Class.from<A>() ], new Handler<A>(target), Class.of(target).getLinker())
    testAIfaceAImpl(proxyObj as A, 1.2, 100, -1.1)
    testA1IfaceAImpl(proxyObj as A1)
    testA2IfaceAImpl(proxyObj as A2)
}

function singleInterfaceTargetsChain() {
    let target = new AImplChild
    let proxyObj = createProxy([ Class.from<A>() ], new Handler<A>(target), Class.of(target).getLinker())
    testAIfaceAImpl(proxyObj as A, 42, 100, -1.1)
    testA1IfaceAImpl(proxyObj as A1)
    testA2IfaceAImpl(proxyObj as A2)
}

function testD2IFace(proxyObj: D2, val1: double, val2: double, val3: double) {
    arktest.assertEQ(proxyObj.foo(new Derived), val1)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.boo(new Derived, val2), val2)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.goo(new Derived).value, val3)
    testIsLastEventMethod()
}

function singleInterfaceTest2() {
    let target = new DImpl
    let proxyObj = createProxy([ Class.from<D2>() ], new Handler<D2>(target), Class.of(target).getLinker())
    testD2IFace(proxyObj as D2, -511, 10000, 3)
}

function testB1Iface(proxyObj: B1) {
    arktest.assertEQ(proxyObj.foo(new Base), 2.2)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(new Derived), 2.2)
    testIsLastEventMethod()
}

function testB2Iface(proxyObj: B2) {
    arktest.assertEQ(proxyObj.foo(new Derived), 2.2)
    testIsLastEventMethod()
}

function multipleTargets1() {
    let target1 = new B1Impl
    let target2 = new B2Impl

    let handler = new DualTargetsHandler<B1, B2>(target1, target2, Class.from<B1>(), Class.from<B2>())
    let proxyObj = createProxy([ Class.from<B1>(), Class.from<B2>() ], handler, Class.of(target1).getLinker())

    // Proxy take only subtype foo, so in both cases B1.foo will passed in handler, and then traversed by handler to B1Impl
    testB1Iface(proxyObj as B1)
    testB2Iface(proxyObj as B2)
}

function testC1Iface(proxyObj: C1) {
    arktest.assertEQ(proxyObj.foo("test"), 2.2)
    testIsLastEventMethod()
}

function testC2Iface(proxyObj: C2) {
    arktest.assertEQ(proxyObj.foo(123), 3.3)
    testIsLastEventMethod()
}

function multipleTargets2() {
    let target1 = new C1Impl
    let target2 = new C2Impl

    let handler = new DualTargetsHandler<C1, C2>(target1, target2, Class.from<C1>(), Class.from<C2>())
    let proxyObj = createProxy([ Class.from<C1>(), Class.from<C2>() ], handler, Class.of(target1).getLinker())

    testC1Iface(proxyObj as C1)
    testC2Iface(proxyObj as C2)
}

function testE4Iface(proxyObj: E4) {
    arktest.assertEQ(proxyObj.foo(new Base), 777.111)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(new Derived), 777.111)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1000), 444.111)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(c'f'), 777.222)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1000.1), 111.222)
    testIsLastEventMethod()
}

function testE6Iface(proxyObj: E6) {
    arktest.assertEQ(proxyObj.foo(new Base), 777.111)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(new Derived), 777.111)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1000), 111.222)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1000.1), 111.222)
    testIsLastEventMethod()
}

function singleTarget1() {
    let target = new EImpl
    let proxyObj = createProxy([ Class.from<E4>(), Class.from<E6>() ], new Handler<EImpl>(target), Class.of(target).getLinker())
    testE4Iface(proxyObj as E4)
    testE6Iface(proxyObj as E6)
}

function testN1Iface(proxyObj: N1) {
    arktest.assertEQ(proxyObj.foo(5), 9.9)
    testIsLastEventMethod()
    proxyObj.a = 15
    testIsLastEventSetter()
    arktest.assertEQ(proxyObj.a, 15)
    testIsLastEventGetter()
}

function testN2Iface(proxyObj: N2) {
    arktest.assertEQ(proxyObj.foo(5), 9.9)
    testIsLastEventMethod()
    proxyObj.b = 25
    testIsLastEventSetter()
    arktest.assertEQ(proxyObj.b, 25)
    testIsLastEventGetter()
}

function testN3Iface(proxyObj: N3) {
    arktest.assertEQ(proxyObj.boo(new Derived), 30.0)
    testIsLastEventMethod()
}

function singleTarget2() {
    let target = new NTarget
    let proxyObj = createProxy([ Class.from<N1>(), Class.from<N2>(), Class.from<N3>() ],
                               new Handler<NTarget>(target),
                               Class.of(target).getLinker())
    testN1Iface(proxyObj as N1)
    testN2Iface(proxyObj as N2)
    testN3Iface(proxyObj as N3)
}

function testQ1Iface(proxyObj: Q1) {
    arktest.assertEQ(proxyObj.foo(new Base), 9.9)
    testIsLastEventMethod()
    proxyObj.a = 15
    testIsLastEventSetter()
    arktest.assertEQ(proxyObj.a, 15)
    testIsLastEventGetter()
}

function testQ2Iface(proxyObj: Q2) {
    arktest.assertEQ(proxyObj.foo(new Base), 9.9)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.goo(new Derived), 20.0)
    testIsLastEventMethod()
    proxyObj.b = 25
    testIsLastEventSetter()
    arktest.assertEQ(proxyObj.b, 25)
    testIsLastEventGetter()
}

function testQ3Iface(proxyObj: Q3) {
    arktest.assertEQ(proxyObj.zoo(), "QTarget")
    testIsLastEventMethod()
}

function singleTarget3() {
    let target = new QTarget
    let proxyObj = createProxy([ Class.from<Q1>(), Class.from<Q2>(), Class.from<Q3>() ],
                               new Handler<QTarget>(target),
                               Class.of(target).getLinker())
    testQ1Iface(proxyObj as Q1)
    testQ2Iface(proxyObj as Q2)
    testQ3Iface(proxyObj as Q3)
}

function testT1Iface(proxyObj: T1) {
    arktest.assertEQ(proxyObj.foo(5), 9.9)
    testIsLastEventMethod()
    proxyObj.a = 15
    testIsLastEventSetter()
    arktest.assertEQ(proxyObj.a, 15)
    testIsLastEventGetter()
}

function testT2Iface(proxyObj: T2) {
    arktest.assertEQ(proxyObj.foo(5), 9.9)
    testIsLastEventMethod()
    proxyObj.b = 25
    testIsLastEventSetter()
    arktest.assertEQ(proxyObj.b, 25)
    testIsLastEventGetter()
}

function testT3Iface(proxyObj: T3) {
    arktest.assertEQ(proxyObj.boo(), 30)
    testIsLastEventMethod()
}

function singleTarget4() {
    let target = new TTarget
    let proxyObj = createProxy([ Class.from<T1>(), Class.from<T2>(), Class.from<T3>() ],
                               new Handler<TTarget>(target),
                               Class.of(target).getLinker())
    testT1Iface(proxyObj as T1)
    testT2Iface(proxyObj as T2)
    testT3Iface(proxyObj as T3)
}

function testF6IFace(proxyObj: F6, val1: Int, val2: Int, val3: Int) {
    arktest.assertEQ(proxyObj.foo(new Derived), val1)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.boo(new Derived, val2), val2)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.goo(new Derived).value, val3)
    testIsLastEventMethod()
    proxyObj.i = val1
    testIsLastEventSetter()
    arktest.assertEQ(proxyObj.i, val1)
    testIsLastEventGetter()
}

function singleInterfaceTest3() {
    let target = new FImpl
    let proxyObj = createProxy([ Class.from<F6>() ], new Handler<F6>(target), Class.of(target).getLinker())
    testF6IFace(proxyObj as F6, -511, 10000, 3)
}

function testI7IFace(proxyObj: I7, val1: Int, val2: Int, val3: Int, val4: Int, val5: Int, val6: Int) {
    arktest.assertEQ(proxyObj.foo(), val1)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1), val2)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1, 1), val3)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1, 1, 1), val4)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1, 1, 1, 1), val5)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1, 1, 1, 1, 1), val6)
    testIsLastEventMethod()
    proxyObj.i = val1
    testIsLastEventSetter()
    arktest.assertEQ(proxyObj.i, val1)
    testIsLastEventGetter()
}

function singleInterfaceTest4() {
    let target1 = new IImpl1()
    let proxyObj1 = createProxy([ Class.from<I7>() ], new Handler<I7>(target1), Class.of(target1).getLinker())
    testI7IFace(proxyObj1 as I7, 1, 2, 3, 4, 5, 6)

    let target2 = new IImpl2()
    let proxyObj2 = createProxy([ Class.from<I7>() ], new Handler<I7>(target2), Class.of(target2).getLinker())
    testI7IFace(proxyObj2 as I7, 1, 21, 3, 41, 5, 6)
}

function testG5IFace(proxyObj: G5, val1: double, val2: double, val3: double, val4: double, val5: double) {
    arktest.assertEQ(proxyObj.foo(new Derived, new Array<number>()), val1)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1), val2)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1, 1), val3)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1, 1, 1), val4)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(), val5)
    testIsLastEventMethod()
}

function singleInterfaceTest5() {
    let target = new GImpl
    let proxyObj = createProxy([ Class.from<G5>() ], new Handler<G5>(target), Class.of(target).getLinker())
    testG5IFace(proxyObj as G5, 22, 2, 5, 6, 4)
}

function testH7IFace(proxyObj: H7, val1: double, val2: double, val3: double, val4: double, val5: double) {
    arktest.assertEQ(proxyObj.foo(1), val1)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1, 1), val2)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1, 1, 1), val3)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(new Derived), val4)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(new Derived(), 1), val5)
    testIsLastEventMethod()
}

function singleInterfaceTest6() {
    let target = new HImpl
    let proxyObj = createProxy([ Class.from<H7>() ], new Handler<H7>(target), Class.of(target).getLinker())
    testH7IFace(proxyObj as H7, 1, 2, 3, 4, 5)
}

function testH7IFace(proxyObj: J6, val1: Int, val2: Int, val3: Int, val4: Int) {
    arktest.assertEQ(proxyObj.foo(1), val1)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1, 1), val2)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1, 1, 1), val3)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(new Derived), val4)
    testIsLastEventMethod()
}

function singleInterfaceTest7() {
    let target = new JImpl
    let proxyObj = createProxy([ Class.from<J6>() ], new Handler<J6>(target), Class.of(target).getLinker())
    testH7IFace(proxyObj as J6, 2, 4, 3, 5)
}

function testK4IFace(proxyObj: K4, val1: Int, val2: Int) {
    arktest.assertEQ(proxyObj.foo(1), val1)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(new Base()), val2)
    testIsLastEventMethod()
    proxyObj.a = val1
    testIsLastEventSetter()
    proxyObj.c = val1
    testIsLastEventSetter()
    arktest.assertEQ(proxyObj.a, val1)
    testIsLastEventGetter()
    arktest.assertEQ(proxyObj.c, val1)
    testIsLastEventGetter()
}

function singleInterfaceTest8() {
    let target = new KImpl
    let proxyObj = createProxy([ Class.from<K4>() ], new Handler<K4>(target), Class.of(target).getLinker())
    testK4IFace(proxyObj as K4, 1, 2)
}

function testL6Face(proxyObj: L6, val1: Int, val2: Int, val3: Int) {
    arktest.assertEQ(proxyObj.foo("1"), val1)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(val2), val2)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(3.0), val3)
    testIsLastEventMethod()
    proxyObj.a = val1
    testIsLastEventSetter()
    arktest.assertEQ(proxyObj.a, val1)
    testIsLastEventGetter()
}

function singleInterfaceTest9() {
    let target = new LImpl
    let proxyObj = createProxy([ Class.from<L6>() ], new Handler<L6>(target), Class.of(target).getLinker())
    testL6Face(proxyObj as L6, 4, 2, 3)
}

function testL6Face(proxyObj: M7, val1: Int, val2: Int, val3: Int, val4: Int) {
    arktest.assertEQ(proxyObj.foo("1"), val1)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(new Derived()), val2)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(val3), val3)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1.0), val4)
    testIsLastEventMethod()
}

function singleInterfaceTest10() {
    let target = new MImpl
    let proxyObj = createProxy([ Class.from<M7>() ], new Handler<M7>(target), Class.of(target).getLinker())
    testL6Face(proxyObj as M7, 1, 2, 3, 4)
}

function testL4Iface(proxyObj: L4, val1: Int) {
    arktest.assertEQ(proxyObj.foo("foo"), val1)
    testIsLastEventMethod()
}

function testL5Iface(proxyObj: L5, val1: Int, val2: Int, val3: Int) {
    arktest.assertEQ(proxyObj.foo("foo"), val1)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1), val2)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1.0), val3)
    testIsLastEventMethod()
}

function multipleInterfacesTest3() {
    let target1 = new LImpl2
    let target2 = new LImpl3

    let handler = new DualTargetsHandler<L5, L4>(target2, target1, Class.from<L5>(), Class.from<L4>())
    let proxyObj = createProxy([ Class.from<L5>(), Class.from<L4>() ], handler, Class.of(target1).getLinker())
    testL4Iface(proxyObj as L4, 5)
    testL5Iface(proxyObj as L5, 5, 2, 3)

    let handler1 = new DualTargetsHandler<L4, L5>(target1, target2, Class.from<L4>(), Class.from<L5>())
    let proxyObj1 = createProxy([ Class.from<L4>(), Class.from<L5>() ], handler1, Class.of(target1).getLinker())
    testL4Iface(proxyObj1 as L4, 1)
    testL5Iface(proxyObj1 as L5, 1, 6 , 3)
}

function testO4Iface(proxyObj: O4) {
    arktest.assertEQ(proxyObj.foo(1), 3)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.bar(1), 2)
    testIsLastEventMethod()
}

function testO5Iface(proxyObj: O5) {
    arktest.assertEQ(proxyObj.foo(1), 3)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.bar(1), 2)
    testIsLastEventMethod()
}

function multipleInterfacesTest4() {
    let target1 = new OImpl1
    let target2 = new OImpl2

    let handler = new DualTargetsHandler<O4, O5>(target1, target2, Class.from<O4>(), Class.from<O5>())
    let proxyObj = createProxy([ Class.from<O4>(), Class.from<O5>() ], handler, Class.of(target1).getLinker())
    testO4Iface(proxyObj as O4)
    testO5Iface(proxyObj as O5)
}

function testP4Iface(proxyObj: P4, val1: Int) {
    arktest.assertEQ(proxyObj.foo(1, 1), 1)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1), 2)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1, 1 , new Base()), 6)
    testIsLastEventMethod()
    proxyObj.a = val1
    testIsLastEventSetter()
    arktest.assertEQ(proxyObj.a, val1)
    testIsLastEventGetter()
    proxyObj.b = val1
    testIsLastEventSetter()
    arktest.assertEQ(proxyObj.b, val1)
    testIsLastEventGetter()
}

function testP6Iface(proxyObj: P6, val1: Int) {
    arktest.assertEQ(proxyObj.foo(1, 1), 1)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(1, 1 , new Base()), 6)
    testIsLastEventMethod()
    proxyObj.b = val1
    testIsLastEventSetter()
    arktest.assertEQ(proxyObj.b, val1)
    testIsLastEventGetter()
    proxyObj.c = val1
    testIsLastEventSetter()
    arktest.assertEQ(proxyObj.c, val1)
    testIsLastEventGetter()
}

function multipleInterfacesTest5() {
    let target1 = new PImpl1
    let target2 = new PImpl2

    let handler = new DualTargetsHandler<P4, P6>(target1, target2, Class.from<P4>(), Class.from<P6>())
    let proxyObj = createProxy([ Class.from<P4>(), Class.from<P6>() ], handler, Class.of(target1).getLinker())
    testP4Iface(proxyObj as P4, 1)
    testP6Iface(proxyObj as P6, 2)
}

function testR11Iface(proxyObj: R11, target1: RImpl1) {
    arktest.assertEQ(proxyObj.foo(), 3)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(new Int), 3)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(10.10, new Base), 5)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(10.10, new Derived), 5)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(new Base), 6)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(new Derived), 6)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.bar(), 2)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.bar(new Int), 7)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.bar(new Int, 10), 7)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.buz(), 4)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.buz(123), 4)
    testIsLastEventMethod()

    // #32956
    // let intValue = 10
    // proxyObj.a = new Int(intValue)
    // testIsLastEventSetter()
    // arktest.assertEQ(proxyObj.a, intValue)
    // testIsLastEventGetter()

    // let doubleValue = 10.10
    // proxyObj.b = new Number(doubleValue)
    // testIsLastEventSetter()
    // arktest.assertEQ(proxyObj.b, doubleValue)
    // testIsLastEventGetter()

    arktest.assertEQ(proxyObj.foo(), target1.foo())
    arktest.assertEQ(proxyObj.foo(new Int), target1.foo(new Int))
    arktest.assertEQ(proxyObj.foo(10.10, new Base), target1.foo(10.10, new Base))
    arktest.assertEQ(proxyObj.foo(10.10, new Derived), target1.foo(10.10, new Derived))
    arktest.assertEQ(proxyObj.foo(new Base), target1.foo(new Base))
    arktest.assertEQ(proxyObj.foo(new Derived), target1.foo(new Derived))
    arktest.assertEQ(proxyObj.bar(), target1.bar())
    arktest.assertEQ(proxyObj.bar(new Int), target1.bar(new Int))
    arktest.assertEQ(proxyObj.bar(new Int, 10), target1.bar(new Int, 10))
    arktest.assertEQ(proxyObj.buz(), target1.buz())
    arktest.assertEQ(proxyObj.buz(123), target1.buz(123))

    // #32956
    // intValue = 20
    // proxyObj.a = new Int(intValue)
    // target1.a = new Int(intValue)
    // arktest.assertEQ(proxyObj.a, target1.a)

    // doubleValue = 20.20
    // proxyObj.b = new Number(doubleValue)
    // target1.b = new Number(doubleValue)
    // arktest.assertEQ(proxyObj.b, target1.b)
}

function testR12Iface(proxyObj: R12, target2: RImpl2) {
    arktest.assertEQ(proxyObj.foo(), 3)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(new Int()), 3)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(10.10, new Base()), 5)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(10.10, new Derived()), 5)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(new Base()), 6.0)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(new Derived()), 6.0)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.bar(), 2)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.bar(new Int(1)), 7)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.bar(new Int(1), 10), 7)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.buz(), 4)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.buz(123), 4)
    testIsLastEventMethod()

    // #32956
    // let intValue = 10
    // proxyObj.a = new Int(intValue)
    // testIsLastEventSetter()
    // arktest.assertEQ(proxyObj.a, intValue)
    // testIsLastEventGetter()

    // let doubleValue = 10.10
    // proxyObj.b = new Number(doubleValue)
    // testIsLastEventSetter()
    // arktest.assertEQ(proxyObj.b, doubleValue)
    // testIsLastEventGetter()

    arktest.assertEQ(proxyObj.foo(), target2.foo())
    arktest.assertEQ(proxyObj.foo(new Int()), target2.foo(new Int()))
    arktest.assertEQ(proxyObj.foo(10.10, new Base()), target2.foo(10.10, new Base()))
    arktest.assertEQ(proxyObj.foo(10.10, new Derived()), target2.foo(10.10, new Derived()))
    arktest.assertEQ(proxyObj.foo(new Base()), target2.foo(new Base()))
    arktest.assertEQ(proxyObj.foo(new Derived()), target2.foo(new Derived()))
    arktest.assertEQ(proxyObj.bar(), target2.bar())
    arktest.assertEQ(proxyObj.bar(new Int()), target2.bar(new Int()))
    arktest.assertEQ(proxyObj.bar(new Int(), 10), target2.bar(new Int(), 10))
    arktest.assertEQ(proxyObj.buz(), target2.buz())
    arktest.assertEQ(proxyObj.buz(123), target2.buz(123))

    // #32956
    // intValue = 20
    // proxyObj.a = new Int(intValue)
    // target2.a = new Int(intValue)
    // arktest.assertEQ(proxyObj.a, target2.a)

    // doubleValue = 20.20
    // proxyObj.b = new Number(doubleValue)
    // target2.b = new Number(doubleValue)
    // arktest.assertEQ(proxyObj.b, target2.b)
}

function multipleInterfacesTest6() {
    let target1 = new RImpl1
    let target2 = new RImpl2

    let handler = new DualTargetsHandler<R11, R12>(target1, target2, Class.from<R11>(), Class.from<R12>())
    let proxyObj = createProxy([ Class.from<R11>(), Class.from<R12>() ], handler, Class.of(target1).getLinker())
    testR11Iface(proxyObj as R11, target1)
    testR12Iface(proxyObj as R12, target2)
}

function testR11R12Iface(proxyObj: R11 | R12) {
    arktest.assertEQ(proxyObj.foo(), 3)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.bar(), 2)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(new Int()), 3)
    testIsLastEventMethod()

    // #32956
    // let intValue = 10
    // proxyObj.a = new Int(intValue)
    // testIsLastEventSetter()
    // arktest.assertEQ(proxyObj.a, intValue)
    // testIsLastEventGetter()

    arktest.assertEQ(proxyObj.foo(10.10, new Base()), 5)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(10.10, new Derived()), 5)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(new Base()), 6.0)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.foo(new Derived()), 6.0)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.bar(new Int()), 7)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.bar(new Int(), 10), 7)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.buz(), 4)
    testIsLastEventMethod()
    arktest.assertEQ(proxyObj.buz(123), 4)
    testIsLastEventMethod()

    // #32956
    // let doubleValue = 10.10
    // proxyObj.b = new Number(doubleValue)
    // testIsLastEventSetter()
    // arktest.assertEQ(proxyObj.b, doubleValue)
    // testIsLastEventGetter()
}

function multipleInterfacesTest7() {
    let target1 = new RImpl1
    let target2 = new RImpl2

    let handler = new DualTargetsHandler<R11, R12>(target1, target2, Class.from<R11>(), Class.from<R12>())
    let proxyObj = createProxy([ Class.from<R11>(), Class.from<R12>() ], handler, Class.of(target1).getLinker())
    testR11R12Iface(proxyObj as R11)
    testR11R12Iface(proxyObj as R12)
}

function testS2S3Iface(proxyObj: S2 | S3) {
    let value = 11
    proxyObj.a = value
    testIsLastEventSetter()
    arktest.assertEQ(proxyObj.a, value)
    testIsLastEventGetter()
}

function multipleInterfacesTest8() {
    let target1 = new SImpl1
    let target2 = new SImpl2

    let handler = new DualTargetsHandler<S2, S3>(target1, target2, Class.from<S2>(), Class.from<S3>())
    let proxyObj = createProxy([ Class.from<S2>(), Class.from<S3>() ], handler, Class.of(target1).getLinker())
    testS2S3Iface(proxyObj as S2)
    testS2S3Iface(proxyObj as S3)
}


function main() {
    const suite = new arktest.ArkTestsuite('Proxy test')

    suite.addTest('Single interface', singleInterfaceTest1)
    suite.addTest('Single interface, Target <: SomeClass <: InterfaceToProxy', singleInterfaceTargetsChain)
    suite.addTest('Single interface D2: D2 <: D1', singleInterfaceTest2)
    suite.addTest('Single interface long hierachy chain', singleInterfaceTest3)
    suite.addTest('Single interface I7Impl <: I7 <: I2, I3, I4, I5, I6 <: I1,', singleInterfaceTest4)
    suite.addTest('Single interface Dimond-ish problem', singleInterfaceTest5)
    suite.addTest('Single interface Dimond-ish problem enchanced', singleInterfaceTest6)
    suite.addTest('Single interface J6 <: J3 <: J2 <:J1; J6 <: J5 <: J4 <:J1', singleInterfaceTest7)
    suite.addTest('Single interface with generics', singleInterfaceTest8)
    suite.addTest('Single interface L6 <: L{1-5}; L4 <: L1; L5 <: L{1-3}', singleInterfaceTest9)
    suite.addTest('Single interface Tree hierarchy', singleInterfaceTest10)

    suite.addTest('Multiple interfaces with single target 1', singleTarget1)
    suite.addTest('Multiple interfaces with single target 2 - N simple overrides', singleTarget2)
    suite.addTest('Multiple interfaces with single target 3 - Q default methods', singleTarget3)
    suite.addTest('Multiple interfaces with single target 4 - T multiple interfaces', singleTarget4)

    suite.addTest('Multiple interfaces with multiple targets B1, B2: B2 <: B1 & B2Impl <: B2, B1Impl', multipleTargets1)
    suite.addTest('Multiple interfaces with multiple targets C1, C2: C2 <: C1 & C2Impl <: C2, C1Impl', multipleTargets2)
    suite.addTest('Multiple interfaces with multiple targets L4 <: L1; L5 <: L{1-3}', multipleInterfacesTest3)
    suite.addTest('Multiple interfaces with multiple targets P3 <: P{1-2}; P4 <: P3; P5 <: P3', multipleInterfacesTest4)
    suite.addTest('Multiple interfaces with multiple targets P4 <: P{1-2}; P6 <: P5 <: P{2-3};', multipleInterfacesTest5)
    suite.addTest('Multiple interfaces with multiple targets R, huge hierarchy', multipleInterfacesTest6)

    // #32956
    suite.addTest('Multiple interface, call proxy object by union of interfaces, huge hierarchy', multipleInterfacesTest7)
    // #32956
    // suite.addTest('Multiple interface, call proxy object by union of interfaces', multipleInterfacesTest8)


    return suite.run()
}
