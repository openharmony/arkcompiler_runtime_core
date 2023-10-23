/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_param_04
tags: []
---*/
class A {}

type Margin = A | string

function format(margin: Margin) {
    assert (margin instanceof A)
}

function main(): void {
    format(new A())
    let a: Margin = new A()
    format(a)
}
