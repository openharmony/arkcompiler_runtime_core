/*---
desc: class members
assert: Members inherited from its direct superclass.
tags: [negative, compile-only]
---*/

interface A {
  fee(): int;
}

final class B implements A {
  public fee(): int {
    return 5;
  }
}

function main(): void {
    let x: B = new B();

    assert x.fee() == 5;
}
