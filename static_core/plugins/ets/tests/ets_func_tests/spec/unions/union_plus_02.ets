/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_plus_02
tags: []
---*/
type Margin = number | string

function main(): void {
    let a: Margin = "5"
    let b: Margin = " px"
    let c: Margin = a + b
    assert (c as string) == "5 px"
}
