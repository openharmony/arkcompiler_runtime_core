/*---
desc: instanceof operator with object.
---*/

class A {}

function main(): void {
    let a: A = new A();
    assert a instanceof A;
}
