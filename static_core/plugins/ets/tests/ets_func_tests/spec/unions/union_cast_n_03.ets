/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_cast_n_03
tags: [negative, compile-only]
---*/
type Margin = number | string
type Padding = number | string

function main(): void {
    let a: Margin = 5
    let c: Padding = a as string // can be CTE as `a` effectively contains number
}
