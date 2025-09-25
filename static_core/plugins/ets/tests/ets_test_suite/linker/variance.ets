/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

class StubError extends Error { }

function stub() {
    throw new StubError();
}
function stubt<T>(): T {
    stub();
    return new Object() as T;
}
function assert_stub(f: () => void) {
    try {
        f();
    } catch (e) {
        if (e instanceof StubError) {
            return;
        }
    }
    arktest.assertTrue(false, "error expected");
}

class T0 { }
class T1 extends T0 { }
class T2 extends T1 { }

// NOTE(vpukhov): cover the rest subtyping cases
class A1 {
    p1(a: T2): void { stub() }
    n1(a: T1): void { stub() }
    p2(): T1 { return stubt<T1>() }
    // n2(): T2 { return stubt<T2>() }
}
class A2 extends A1 {
    static create(): A1 { return new A2() }

    n1(a: T2): void { }
    p1(a: T1): void { }
    p2(): T2 { return new T2() }
    // n2(): T1 { return new T1() }
}
function simple() {
    let a: A1 = A2.create();
    let t1 = new T1();
    let t2 = new T2();
    assert_stub(() => { stub() })

    a.p1(t2); assert_stub(() => { a.n1(t1) });
    a.p2();
}

class B1 {
    p1(a: B2): void { stub() }
    n1(a: B1): void { stub() }
    p2(): B1 { return stubt<B1>() }
}
class BInterm extends B1 { }
class B2 extends BInterm {
    static create(): B1 { return new B2() }

    n1(a: B2): void { }
    p1(a: B1): void { }
    p2(): B2 { return new B2() }
}
function loops() {
    let b: B1 = B2.create();
    let b1 = new B1();
    let b2 = new B2();
    assert_stub(() => { stub() })

    b.p1(b2); assert_stub(() => { b.n1(b1) });
    b.p2();
}

class C1 {
    m(a: T2): void { stub() }
    m(a: T1): void { stub() }
    m(a: T0): void { stub() }
}
class C2 extends C1 {
    static create(): C1 { return new C2() }

    m(a: T1): void { }
}
function overrideMultiple() {
    let c: C1 = C2.create();

    assert_stub(() => { c.m(new T0()) });
    c.m(new T1());
    c.m(new T2());
}

class D1 {
    m(a: FixedArray<string> | T1): void { stub() }
}
class D2 extends D1 {
    m(a: FixedArray<object> | T0): void { }
}
function overrideUnionArrays() {
    let d = new D2()
    let arr: FixedArray<string> = new FixedArray<string>(3)
    d.m(arr)
}

class E1 {
    m(a: FixedArray<string>) { stub() }
}
class E2 extends E1 {
    static create(): E1 { return new E2() }
    m(a: FixedArray<FixedArray<Date> | string | number>): void { }
}
function overrideFixedArrayUnion1() {
    let e: E1 = E2.create();
    const fa: FixedArray<string> = ['s', '1']
    e.m(fa)
}

class F1 {
    m(a: FixedArray<string | number>) { stub() }
}
class F2 extends F1 {
    static create(): F1 { return new F2() }
    m(a: FixedArray<Object>): void { }
}
function overrideFixedArrayUnion2() {
    let f: F1 = F2.create();
    const fa: FixedArray<string | number> = ['s', 1]
    f.m(fa)
}

class G1 {
    m(a: Object | null): void { }
}

class G2 extends G1 {
    static create(): G1 { return new G2() }
    m(a: Object ) { stub() }
}

function notOverrideUnionWithNull1() {
    let g1: G1 = G2.create();
    let g2: G2 = new G2()
    g1.m(null)
    g2.m(null)
}

class H1 {
    m<T>(a: T): void { }
}

class H2 extends H1 {
    static create(): H1 { return new H2() }
    m(a: Object ) { stub() }
}

function notOverrideUnionWithNull2() {
    let h1: H1 = H2.create();
    let h2: H2 = new H2()
    h1.m(null)
    h2.m(null)
}

function main() {
    simple();
    loops();
    overrideMultiple();
    overrideUnionArrays();
    overrideFixedArrayUnion1();
    overrideFixedArrayUnion2();
    notOverrideUnionWithNull1();
    notOverrideUnionWithNull2();
}
