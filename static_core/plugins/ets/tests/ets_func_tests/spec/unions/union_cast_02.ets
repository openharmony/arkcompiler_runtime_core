/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_cast_02
tags: []
---*/
type Margin = number | string

function main(): void {
    let a3: number = 5
    let b3: Margin = a3 as Margin // should work
    assert b3 == 5
}
