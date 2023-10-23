/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_return_03
tags: []
---*/
type Margin = number | string

function format_number(): Margin {
    return 5.5
}

function main(): void {
    let b: number = format_number() as number
    assert b == 5.5
}
