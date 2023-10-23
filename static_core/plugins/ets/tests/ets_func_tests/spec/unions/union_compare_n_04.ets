/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_compare_n_04
tags: [negative, compile-only]
---*/
type Margin = number | string

class A {}

function main(): int {
    let a: Margin = "5 px"
    let b: A = new A()
    if (a <= b) { // expected CTE
        return -1
    }
    return 0
}
