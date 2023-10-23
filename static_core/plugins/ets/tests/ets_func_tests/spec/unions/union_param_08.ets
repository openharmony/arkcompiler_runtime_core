/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_param_06
tags: []
---*/
class A {}
class B extends A {}

type Margin = B | string

function callB(b: B) {
    assert b instanceof B
}

function callA(a: Margin) {
    if (a instanceof B){
        callB(a as B)
    }
}

function main(): void {
    callA(new B())
    let a: Margin = new B()
    callA(a)
}
