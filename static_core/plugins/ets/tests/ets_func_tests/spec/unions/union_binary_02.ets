/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_binary_02
tags: []
---*/
type Margin = number | string

function main(): void {
    let a: Margin = 7
    let b: Margin = 5
    let c: Margin = a % b
    assert (c as number) == 2
}
