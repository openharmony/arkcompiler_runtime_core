/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_assignment_06
tags: []
---*/
type Margin = number | string
type Padding = number | string

function main(): void {
    let a: Margin = 5.5
    let b: Padding = a // ok
}
