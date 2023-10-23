/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_return_n_06
tags: [negative, compile-only]
---*/
type Margin = number | string

class A {}

function format_number(): Margin {
    return 5
}

function format_string(): Margin {
    return "5 px"
}

function main(): void {
    let a: A = format_number()
}
