/*---
desc: Class initializer with throws clause.
assert: A class initializer cannot throw any exception and panic, as it effectively is a non-throwing function, see “Non-throwing Functions”.
tags: [compile-only, negative]
---*/

class A {
  public x: int;

  static throws { 
    this.x = 5;
  }
}