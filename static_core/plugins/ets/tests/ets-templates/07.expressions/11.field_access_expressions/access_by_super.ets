/*---
desc: Access to field by keyword super.
assert: A field access expression may access a field of an object which is referenced by the value of either an expression or the special keyword super.
---*/

class A {
  public x: int;
}

final class B extends A {
  public bar(): int {
    return super.x;
  }
}

function main(): void {
    let instance: B = new B();

    assert instance.bar() == 0;
}
