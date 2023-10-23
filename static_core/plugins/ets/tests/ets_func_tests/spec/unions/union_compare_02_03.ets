/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_compare_02_03
tags: []
---*/

class A {}
class B {}

type Union1 = B | string

function main(): int {
    let a: Union1 = "hello"
    let b: A = new A()
    if (a === b) {
        return -1
    }
    return 0
}
