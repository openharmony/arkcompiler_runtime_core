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
desc: >-
    Subtyping for interfaces with combined in/out type parameters. For interface I<in T1, out T2>,
    if S>:U and T<:V then I<S,T> <: I<U,V> (15.2.2).
---*/

interface Transformer<in I, out O> {
    transform(input: I): O
}

class StrToNumber implements Transformer<String, Number> {
    override transform(input: String): Number {
        return input.length
    }
}

class HD {}
class HE extends HD {}
class HF extends HE {}

interface Proc<in T, out S> {
    run(t: T): S
}

class ProcEE implements Proc<HE, HE> {
    override run(t: HE): HE { return new HE() }
}

function applyTransformer(t: Transformer<String, Object>, input: String): Object {
    return t.transform(input)
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Combined in/out subtyping for interfaces")

    suite.addTest("Transformer<String, Number> <: Transformer<String, Object>", () => {
        let t: Transformer<String, Object> = new StrToNumber()
        let result: Object = t.transform("hi")
        arktest.assertTrue(result instanceof Number)
        arktest.assertEQ(result.toString(), "2")
    })

    suite.addTest("Proc<HE, HE> <: Proc<HF, HD>: contravariant T widens, covariant S narrows", () => {
        let p: Proc<HE, HE> = new ProcEE()
        let q: Proc<HF, HD> = p
        let r: HD = q.run(new HF())
        arktest.assertTrue(r instanceof HE)
    })

    suite.addTest("function call passes Transformer<String, Number> as Transformer<String, Object>", () => {
        let result: Object = applyTransformer(new StrToNumber(), "hello")
        arktest.assertTrue(result instanceof Number)
        arktest.assertEQ(result.toString(), "5")
    })

    return suite.run()
}
