/*---
desc: Inheritance static method from superinterfaces.
assert: A class does not inherit private or static methods from its superinterfaces.
tags: [negative, compile-only]
---*/

interface A {
  static foo(): int {
    return 1;
  }
}

class B implements A {
  fee(): int {
    return foo();
  }
}

function main(): void {
  let instance: B = new B();

  assert instance.fee() == 1;
}
