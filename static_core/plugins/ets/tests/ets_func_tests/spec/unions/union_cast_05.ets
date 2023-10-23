/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_cast_05
tags: []
---*/
type Margin = number | string

function main(): void {
    let a1: Margin = 5
    let b1: byte = a1 as byte
    assert b1 == 5
}
