/*---
desc: Access to object field
---*/

class A {
    public x: int = 5;
}

function main(): void {
    let a: A = new A();
    assert a.x == 5;
}
