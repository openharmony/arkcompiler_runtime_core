/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_return_04
tags: []
---*/
type Margin = number | string

function format_number(): Margin {
    return 5.5
}

function main(): void {
    let c: Margin = format_number()
    assert c as number == 5.5
}
