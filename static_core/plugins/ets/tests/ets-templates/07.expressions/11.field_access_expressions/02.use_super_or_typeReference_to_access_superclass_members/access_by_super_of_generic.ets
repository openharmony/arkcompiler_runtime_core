/*---
desc: Access to field by keyword super.
assert: The form T.super.identifier refers to the field named identifier of the lexically enclosing instance corresponding to T, but with that instance viewed as an instance of the superclass of T.
---*/

class A {
  public x: int;
}

class B extends A{
  public y: int;
}

final class C<T> extends A {
  public bar(): int {
    return T.super.x;
  }
}

function main(): void {
    let instance: C<B> = new C<B>();

    assert instance.bar() == 1;
}
