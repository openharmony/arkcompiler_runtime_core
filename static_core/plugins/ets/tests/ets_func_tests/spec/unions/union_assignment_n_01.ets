/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_assignment_n_01
tags: [negative, compile-only]
---*/
class A {}
type ANumber = A | number

function main(): void {
    let a: ANumber = new A()
    a = "5" // expected CTE
}
