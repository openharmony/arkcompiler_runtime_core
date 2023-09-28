/*---
desc: Inheritance of private methods.
assert: The interface inherits, from the interfaces it extends, all members of those interfaces, except for private methods.
tags: [compile-only, negative]
---*/

interface A {
  private m(): int { return 5; }
}

class B implements A {
  public static bar(): int {
    return m();
  }
}