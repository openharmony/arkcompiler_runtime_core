/*---
desc: Class initializer with super keyword.
assert: It is a compile-time error if the keyword this or the keyword super or any type variable declared outside the class initializer, appears anywhere within a class initializer.
tags: [compile-only, negative]
---*/

class A {
  public foo(): int {
    return 5;
  }
}

class B extends A {
  public static x: int;

  static {
    x = 5 + super.foo();
  }
}
