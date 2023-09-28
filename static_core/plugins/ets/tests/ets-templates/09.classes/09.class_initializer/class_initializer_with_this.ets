/*---
desc: Class initializer with this keyword.
assert: It is a compile-time error if the keyword this or the keyword super or any type variable declared outside the class initializer, appears anywhere within a class initializer.
tags: [compile-only, negative]
---*/

class A {
  public x: int;

  static { 
    this.x = 5;
  }
}