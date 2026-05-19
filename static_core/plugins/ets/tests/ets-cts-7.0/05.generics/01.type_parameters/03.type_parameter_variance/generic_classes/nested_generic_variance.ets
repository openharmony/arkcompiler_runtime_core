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
desc: Variance composition through nested generic instantiations follows from transitive subtyping.
---*/

class CovBox<out T> {
    readonly v: T
    constructor(v: T) { this.v = v }
}

class ContraSink<in T> {
    private callCount: int = 0
    accept(p: T): void { this.callCount += 1 }
    getCallCount(): int { return this.callCount }
}

class CovOuter<out U> {
    readonly b: U
    constructor(b: U) { this.b = b }
}

class CovWrapper<out T> {
    readonly fn: () => T
    constructor(fn: () => T) { this.fn = fn }
}

class CovProducer<out T> {
    readonly item: T
    constructor(item: T) { this.item = item }
}

class MixPipeline<in I, out O> {
    readonly prod: CovProducer<O>
    constructor(p: CovProducer<O>) { this.prod = p }
    consume(input: I): void {}
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Nested generic variance composition")

    suite.addTest("covariant of covariant: CovOuter<CovBox<String>> <: CovOuter<CovBox<Object>>", () => {
        let o: CovOuter<CovBox<Object>> = new CovOuter<CovBox<String>>(new CovBox<String>("x"))
        arktest.assertTrue(o.b.v instanceof String)
        arktest.assertEQ(o.b.v.toString(), "x")
    })

    suite.addTest("covariant of contravariant: CovOuter<ContraSink<Object>> <: CovOuter<ContraSink<String>>", () => {
        let inner: ContraSink<Object> = new ContraSink<Object>()
        let o: CovOuter<ContraSink<String>> = new CovOuter<ContraSink<Object>>(inner)
        o.b.accept("hello")
        o.b.accept("world")
        arktest.assertEQ(inner.getCallCount(), 2)
    })

    suite.addTest("covariant wrapper around function returning T", () => {
        let w: CovWrapper<Object> = new CovWrapper<String>((): String => "x")
        arktest.assertEQ(w.fn(), "x")
    })

    suite.addTest("multi-level: MixPipeline<Object, String> <: MixPipeline<String, Object>", () => {
        let p: MixPipeline<String, Object> = new MixPipeline<Object, String>(new CovProducer<String>("x"))
        arktest.assertTrue(p.prod.item instanceof String)
        arktest.assertEQ(p.prod.item.toString(), "x")
    })

    return suite.run()
}
