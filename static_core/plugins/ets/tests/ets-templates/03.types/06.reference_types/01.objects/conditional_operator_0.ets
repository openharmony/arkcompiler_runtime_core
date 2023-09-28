/*---
desc: Class instance in conditional true operator.
---*/

class A {}
class B {}

function main(): void {
    let a: A = new A();
    let b: B = new B();
    let cond = true;
    let o = cond ? a : b;
    
    assert o instanceof A;
}
