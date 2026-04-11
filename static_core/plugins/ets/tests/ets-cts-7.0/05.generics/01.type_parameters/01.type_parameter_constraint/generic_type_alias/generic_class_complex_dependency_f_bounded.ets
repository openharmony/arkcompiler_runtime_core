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
desc: Complex generic type dependencies (F-Bounded).
params: >-
    A<T extends U<T>>
---*/

class U<S> {
    field0: S
    constructor(v: S){
        this.field0 = v
    }
}

class A<T extends U<T>> {
    fieldA: T
    constructor(v: T){
        this.fieldA = v;
    }
}

final class Af<T extends U<T>> {
    fieldA?: T = undefined
}

abstract class Aa<T extends U<T>> {
    fieldA: T
    constructor(v: T){
        this.fieldA = v;
    }
}

class B extends U<B> {
    constructor(){
        super(this)
    }
}

class C extends Aa<B> {
    constructor(v: B){
        super(v)
    }
}

function testClassGeneric() {
    let f = new B()
    let a = new A<B>(f)
    arktest.assertTrue(a instanceof A)
    arktest.assertTrue(a.fieldA instanceof B)
    arktest.assertEQ(a.fieldA, f)
    arktest.assertTrue(a.fieldA.field0 instanceof B)
    arktest.assertEQ(a.fieldA.field0, f)
}

function testClassFinalGeneric() {
    let a = new Af<B>()
    a.fieldA = new B()
    arktest.assertTrue(a instanceof Af)
    arktest.assertTrue(a.fieldA instanceof B)
    arktest.assertTrue((a.fieldA as B).field0 instanceof B)
    arktest.assertEQ((a.fieldA as B).field0, a.fieldA)
}

function testClassAbstractGeneric() {
    let f = new B()
    let a = new C(f)
    arktest.assertTrue(a instanceof C)
    arktest.assertTrue(a.fieldA instanceof B)
    arktest.assertEQ(a.fieldA, f)
    arktest.assertTrue(a.fieldA.field0 instanceof B)
    arktest.assertEQ(a.fieldA.field0, f)
}

function main(): int {
    const testSuite = new arktest.ArkTestsuite("generic_class_complex_dependency_f_bounded");
    testSuite.addTest("testClassGeneric", testClassGeneric)
    testSuite.addTest("testClassFinalGeneric", testClassFinalGeneric)
    testSuite.addTest("testClassAbstractGeneric", testClassAbstractGeneric)
    return testSuite.run()
}
