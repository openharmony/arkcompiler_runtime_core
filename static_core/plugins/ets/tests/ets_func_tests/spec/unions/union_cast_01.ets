/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_cast_01
tags: []
---*/
type Margin = number | string
type Padding = number | string

function main(): void {
    let a1: Margin = "5 px"
    let b1: Padding = a1 as string // should work
    assert b1 as string == "5 px"

    let a2: Margin = 5.5
    let b2: Padding = a2 as number // should work
    assert b2 as number == 5.5

    let a3: Margin = 5
    let b3: Padding = a3 as number // should work
    assert b3 as number == 5
}
