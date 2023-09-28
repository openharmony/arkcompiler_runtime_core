/*---
desc: Overriding method of superclass.
assert: One of the following is true. mA is public. mA is protected. mA is declared with package access in the same package as C, and either C declares mC or mA is a member of the direct superclass of C. mA is declared with package access and mC overrides mA from some superclass of C.
tags: [compile-only, negative]
---*/

class A {
  private m(): int {
    return 1;
  }
}

final class C extends A {
  override m(): int {
    return 5;
  }
}
