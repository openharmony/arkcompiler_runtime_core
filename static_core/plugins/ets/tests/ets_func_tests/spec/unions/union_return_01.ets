/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_return_01
tags: []
---*/
type Margin = number | string

function format_string(): Margin {
    return "5 px"
}

function main(): void {
    let d: Margin = format_string()
    assert d as string == "5 px"
}
