/*---
desc: Inherit multiple methods with override-equivalent signatures.
assert: It is a compile-time error if a class C inherits a concrete method whose signature is override-equivalent with another method inherited by C.
tags: [compile-only, negative]
---*/

class A {
  public m(): int {
    return 1;
  }
}

interface I {
  m(): int {
    return 1;
  }
}

final class C extends A {
  override m(): int {
    return 5;
  }
}
