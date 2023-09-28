/*---
desc: Access to field
assert: A field access expression may access a field of an object which is referenced by the value of either an expression or the special keyword super.
---*/

class A {
  public x: int;
}

function main(): void {
    let instance: A = new A();

    assert instance.x == 0;
}
