/*---
desc: Class initializer with return statement.
assert: It is a compile-time error if a return statement appears anywhere within a class initializer.
tags: [compile-only, negative]
---*/

class A {
  public static x: int;

  static { 
    x = 5;
    return true;
  }
}