/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

class X {
    static baz() {}
    foo() {}
}

function bar() {}

function testFunction() {
    let bar1 = bar
    let bar2 = bar
    assertEQ(bar1, bar2)
}

function testInstanceMethod() {
    let x1 = new X()
    let x2 = new X()
    assertTrue(x1 !== x2)
    let foo1 = x1.foo
    let foo2 = x1.foo
    let foo3 = x2.foo
    let baz1 = X.baz
    assertEQ(foo1, foo2)
    assertNE(foo1, foo3)
    assertNE(foo1, baz1)
}

function testStaticMethod() {
    let baz1 = X.baz
    let baz2 = X.baz
    assertEQ(baz1, baz2)
}

function testArrowFunction() {
    let c : Function | undefined = undefined
    for (let i = 0; i < 2; i++) {
        let d = ()=>{}
        if (c != undefined) {
            assertNE(c, d)
        }
        c = d
    }
}

function main() {
    const suite = new ArkTestsuite('Function Compare tests')
    suite.addTest('Function Compare', testFunction)
    suite.addTest('Instance Method Compare', testInstanceMethod)
    suite.addTest('Static Method Compare', testStaticMethod)
    suite.addTest('Arrow Function Compare', testArrowFunction)
    return suite.run()
}
