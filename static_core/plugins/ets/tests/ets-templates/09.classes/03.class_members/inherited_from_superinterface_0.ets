/*---
desc: class members
assert: Members inherited from any direct superinterface.
---*/

interface A {
  foo: int = 5;
}

final class B implements A {
}

function main(): void {
    let x: A = new B();    
    assert x.foo == 5;
}
