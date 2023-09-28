/*---
desc: Class initializer with type variable declared outside the class initializer.
assert: It is a compile-time error if the keyword this or the keyword super or any type variable declared outside the class initializer, appears anywhere within a class initializer.
tags: [compile-only, negative]
---*/

class A {
  public a: A;

  static { 
    a = new A();
  }
}