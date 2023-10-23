/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_compare_02_04
tags: []
---*/

class A {}

type Union1 = string | number

function main(): int {
    let a: Union1 = "hello"
    let b: A = new A()
    if (a === b) {
        return -1
    }
    return 0
}
