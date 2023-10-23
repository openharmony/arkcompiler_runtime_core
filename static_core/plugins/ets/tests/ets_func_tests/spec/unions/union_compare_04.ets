/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_compare_04
tags: []
---*/
type Margin = number | string

class A {}

function main(): void {
    let a: Margin = 5.5
    let b: Margin = 6.5
    assert a < b
    assert b > a
    
    let c: Margin = 5.5
    assert a <= c
    assert c >= a
}
