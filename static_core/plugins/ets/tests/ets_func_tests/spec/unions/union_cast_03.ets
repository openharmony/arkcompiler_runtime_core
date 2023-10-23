/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_cast_03
tags: []
---*/
type Margin = number | string

function main(): void {
    let a2: number = 5.5
    let b2: Margin = a2 as Margin // should work
    assert b2 == 5.5
}
