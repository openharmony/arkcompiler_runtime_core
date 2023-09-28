/*---
desc: Inherit multiple methods with override-equivalent signatures.
assert: It is possible for a class to inherit multiple methods with override-equivalent signatures.
tags: [compile-only]
---*/

class A {
  public m(): int {
    return 1;
  }

  public d(): int {
    return 1;
  }

  public c(): int {
    return 1;
  }
}

class C extends A {
  override m(): int {
    return 5;
  }

  override final d(): int {
    return 5;
  }

  override c(): int {
    return 5;
  }
}
