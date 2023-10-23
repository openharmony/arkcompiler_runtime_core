/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_compare_n_06
tags: [negative, compile-only]
---*/
type Margin = number | string

class A {}

function main(): int {
    let a: Margin = "5 px"
    let b: Margin = 5
    if (a <= b) { // expected CTE, a and b effectively different types
        return -1
    }
    return 0
}
