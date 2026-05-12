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
    Variance modifiers combined with non-trivial constraints. Constraint defines an upper bound; variance
    determines the subtyping direction at instantiation. Both must be satisfied.
---*/

class CBase {}
class CDerived extends CBase {}

class CovContainer<out T extends CBase> {
    readonly v: T
    constructor(v: T) { this.v = v }
}

class ContraAcceptor<in T extends CBase> {
    private last: CBase | null = null
    accept(p: T): void { this.last = p }
    getLast(): CBase | null { return this.last }
}

interface RecComparable<T> {
    compareTo(other: T): int
}

class RecSorter<out T extends RecComparable<T>> {
    readonly item: T
    constructor(item: T) { this.item = item }
}

class RecItem implements RecComparable<RecItem> {
    readonly tag: String
    constructor(tag: String) { this.tag = tag }
    compareTo(other: RecItem): int { return 0 }
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Variance with constraints")

    suite.addTest("covariant + constraint: CovContainer<CDerived> <: CovContainer<CBase>", () => {
        let c: CovContainer<CBase> = new CovContainer<CDerived>(new CDerived())
        arktest.assertTrue(c.v instanceof CDerived)
    })

    suite.addTest("contravariant + constraint: ContraAcceptor<CBase> <: ContraAcceptor<CDerived>", () => {
        let a: ContraAcceptor<CDerived> = new ContraAcceptor<CBase>()
        let d = new CDerived()
        a.accept(d)
        arktest.assertEQ(a.getLast(), d)
    })

    suite.addTest("covariant + recursive constraint compiles and runtime works", () => {
        let s: RecSorter<RecItem> = new RecSorter<RecItem>(new RecItem("a"))
        arktest.assertEQ(s.item.tag, "a")
        arktest.assertEQ(s.item.compareTo(new RecItem("b")), 0)
    })

    return suite.run()
}
