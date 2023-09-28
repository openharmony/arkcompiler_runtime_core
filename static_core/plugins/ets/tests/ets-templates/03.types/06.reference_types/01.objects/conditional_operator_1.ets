/*---
desc: Class instance in conditional false operator.
---*/

class A {}
class B {}

function main(): void {
    let a: A = new A();
    let b: B = new B();
    let cond = false;
    let o = cond ? a : b;
    
    assert o instanceof B;
}
