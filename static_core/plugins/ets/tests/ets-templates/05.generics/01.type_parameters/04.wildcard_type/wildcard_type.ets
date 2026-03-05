/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: |-
    Wildcard Type is a type that can be used as a type argument when Generic Instantiations occur to denote an unknown
    instantiation of a invariant Type Parameter. Wildcard Type is a semantic notion that may appear during the Type
    Inference, it can not be represented with ArkTS syntax.
---*/

// based on spec code examples
class X<T> {
    p: T
    constructor(v: T) { this.p = v }
}

function foo(obj: object): boolean {
    if (obj instanceof X) {
        let x = obj // instanceof guarantees that the type is *some* X
        // yet the type parameter is unknown
        // The inferred type of obj is X<*>

        let e = obj.p // the type of expression is the constraint
        // of the corresponding type parameter, which
        // is Any
    }
    return obj instanceof X
}

class C { }

interface X1<T extends C> {
    read(): T
    write(p: T): void
    prop: T
}

class X1Impl implements X1<C> {
    read(): C { return new C }
    write(p: C): void {}
    prop: C = new C
}

function foo1(o: Object): boolean {
    if (o instanceof X1) {
        let p: C = o.read() // OK, T in "out" position is instantiated with C
    }
    return o instanceof X1
}

interface X2<out T extends C> {
    read(): T
}

class X2Impl implements X2<C> {
    read(): C { return new C }
}

function foo2(o: Object) : boolean {
    if (o instanceof X2) { // type of o is X2<C>
        let o1: X2<C> = o // OK, X2<C> is a subtype of X2<C>

        let p: C = o.read() // OK, T was instantiated with C, C assigned to C
    }
    return o instanceof X2
}

interface X3<in T extends C> {
    write(p: T): void
}

class X3Impl implements X3<C> {
    write(p: C): void {}
}

function foo3(o: Object): boolean {
    if (o instanceof X3) { // type of o is X3<never>
        let o2: X3<never> = o // OK, X3<never> is a subtype of X3<never>
    }
	return o instanceof X3
}

class X4<T> {
    p: T
    constructor(p: T) { this.p = p }
}

function foo4(a: Any, b: Any): boolean {
    if (a instanceof X4 && b instanceof X4) {
		let a1 = a // X4<*>
		let a2 = b // X4<*>
		a1 = a2 // OK, inferred X4<*> and X4<*> types are identical
		// Despite concrete type arguments are not known,
		// the code is type-safe
	}
	return a instanceof X4 && b instanceof X4
}

function main() {
    arktest.assertTrue(foo(new X<number>(1)), "expect foo return true")
    arktest.assertTrue(foo1(new X1Impl), "expect foo1 return true")
    arktest.assertTrue(foo2(new X2Impl), "expect foo2 return true")
    arktest.assertTrue(foo3(new X3Impl), "expect foo3 return true")
    arktest.assertTrue(foo4(new X4<number>(1), new X4<string>("a")), "expect foo4 return true")
}
