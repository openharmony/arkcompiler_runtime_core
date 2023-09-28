/*---
desc: Hiding method of superclass.
assert: It is a compile-time error if a static method hides an instance method.
tags: [compile-only, negative]
---*/

class A {
 public m(): int {
    return 1;
  }
}

class C extends A {
  public static m(): int {
    return 5;
  }
}
