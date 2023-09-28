/*---
desc: Overriding method of superclass.
assert: It is a compile-time error if the overridden method, mA, is a static method.
tags: [compile-only, negative]
---*/

class A {
  static m(): int {
    return 1;
  }
}

final class C extends A {
  override m(): int {
    return 5;
  }
}
