/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_assignment_02
tags: []
---*/
class A {}
type ANumber = A | number

function main(): void {
    let a2: ANumber = new A()
    a2 = 5
    assert a2 == 5
}
