/*---
desc: Link equality operator ==.
---*/

class A {}

function main(): void {
    let a: A = new A();
    let b: A = a;
    assert a == b;
}
