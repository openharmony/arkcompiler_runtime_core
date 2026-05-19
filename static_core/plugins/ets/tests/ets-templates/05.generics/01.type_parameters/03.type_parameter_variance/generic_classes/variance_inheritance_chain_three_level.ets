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
desc: Variance modifiers propagate through three-level inheritance chains and combined in/out parameters.
---*/

class CovA<out T> {
    readonly v: T
    constructor(v: T) { this.v = v }
}
class CovB<out T> extends CovA<T> {
    readonly w: T
    constructor(v: T, w: T) { super(v); this.w = w }
}
class CovC<out T> extends CovB<T> {
    constructor(v: T, w: T) { super(v, w) }
}

class ContraA<in T> {
    private acceptCount: int = 0
    accept(p: T): void { this.acceptCount += 1 }
    getAcceptCount(): int { return this.acceptCount }
}
class ContraB<in T> extends ContraA<T> {
    private acceptTwiceCount: int = 0
    acceptTwice(p: T): void { this.acceptTwiceCount += 1 }
    getAcceptTwiceCount(): int { return this.acceptTwiceCount }
}
class ContraC<in T> extends ContraB<T> {}

class MixA<in T, out S> {
    readonly s: S
    constructor(s: S) { this.s = s }
    foo(t: T): S { return this.s }
}
class MixB<in T, out S> extends MixA<T, S> {
    constructor(s: S) { super(s) }
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Multi-level variance inheritance chain")

    suite.addTest("3-level covariant chain: CovC<String> <: CovA<Object>", () => {
        let c: CovA<Object> = new CovC<String>("x", "y")
        arktest.assertTrue(c.v instanceof String)
        arktest.assertEQ(c.v.toString(), "x")
    })

    suite.addTest("3-level contravariant chain: ContraC<Object> <: ContraC<String>", () => {
        let c: ContraC<String> = new ContraC<Object>()
        c.accept("hello")
        c.acceptTwice("again")
        arktest.assertEQ(c.getAcceptCount(), 1)
        arktest.assertEQ(c.getAcceptTwiceCount(), 1)
    })

    suite.addTest("multi-param variance through inheritance: MixB<Object, String> <: MixA<String, Object>", () => {
        let b: MixA<String, Object> = new MixB<Object, String>("x")
        let result: Object = b.foo("input")
        arktest.assertTrue(result instanceof String)
        arktest.assertEQ(result.toString(), "x")
    })

    return suite.run()
}
