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
    For interface Proc<in T, out S> with hierarchy HD -> HE -> HF, Proc<HF,HD> is not a subtype of
    Proc<HE,HD>: contravariant T requires HF>:HE which is false.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0318: Type 'Proc<HF,HD>' cannot be assigned to type 'Proc<HE,HD>'"
---*/

class HD {}
class HE extends HD {}
class HF extends HE {}

interface Proc<in T, out S> {
    run(t: T): S
}

class ProcFD implements Proc<HF, HD> {
    override run(t: HF): HD { return new HD() }
}

function makeFD(): Proc<HF, HD> { return new ProcFD() }

function main(): void {
    let p: Proc<HF, HD> = makeFD()
    let q: Proc<HE, HD> = p
}
