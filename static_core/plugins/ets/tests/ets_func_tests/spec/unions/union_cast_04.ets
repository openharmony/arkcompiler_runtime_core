/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_cast_04
tags: []
---*/
type Margin = number | string

function main(): void {
    let a1: string = "5 px"
    let b1: Margin = a1 as Margin // should work
    assert b1 == "5 px"
}
