/*---
desc: Access to field by nullable reference type
assert: An expression must be a non-nullable reference type T to be used for Field Access. Otherwise, a compile-time error occurs.
tags: [negative]
---*/

class A {
  public x: int;
}

function main(): void {
    let instance: A? = new A();

    assert instance.x == 1;
}
