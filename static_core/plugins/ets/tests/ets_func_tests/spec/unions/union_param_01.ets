/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_param_01
tags: []
---*/
type Margin = number | string

function format(margin: Margin) {
    assert margin as number == 5
}

function main(): void {
    format(5)
}
