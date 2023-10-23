/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_cast_06
tags: []
---*/
type Margin = number | string

function main(): void {
    let a1: Margin = 65536
    let b1: long = a1 as long
    assert b1 == 65536
}
