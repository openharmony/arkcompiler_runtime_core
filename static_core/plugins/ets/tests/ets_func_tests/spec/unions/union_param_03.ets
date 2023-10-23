/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_param_03
tags: []
---*/
type Margin = number | string

function format(margin: Margin){
    assert margin instanceof string
    assert margin as string == "5 px"
}

function main(): void {
    format("5 px")
    let a: Margin = "5 px"
    format(a)
}
