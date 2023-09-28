/*---
desc: Inheritance multiple default method from superinterface with override-equivalent signature.
assert: It is possible for an interface to inherit several methods with override-equivalent signatures (link to signatures).
tags: [compile-only]
---*/

interface A {
  foo(): int {
    return 1;
  }
}

interface B extends A {
  foo(): int {
    return 2;
  }
}

interface C extends C {
  foo(): int {
    return 3;
  }
}

interface D extends A, B, C {
  override foo(): int {
    return 4;
  }
}