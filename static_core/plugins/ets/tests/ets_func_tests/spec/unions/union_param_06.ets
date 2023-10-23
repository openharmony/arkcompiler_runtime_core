/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_param_06
tags: []
---*/
type Margin = number | string

function format(margin: Margin) {
    assert margin as number == 5.5
}

function main(): void {
    let a: Margin = 5.5
    format(a)
}
