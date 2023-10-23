/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_param_n_01
tags: []
---*/
type Margin = number | string

class A {}

function format(margin: Margin){}

function main(): void {
    format(new A()) // expected CTE
}
