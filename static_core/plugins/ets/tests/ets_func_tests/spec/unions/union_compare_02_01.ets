/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_compare_02_01
tags: []
---*/

class A {}
class B {}
class C {}

type Union1 = B | C

function main(): int {
    let a: Union1 = new B()
    let b: A = new A()
    if (a === b) {
        return -1
    }
    return 0
}
