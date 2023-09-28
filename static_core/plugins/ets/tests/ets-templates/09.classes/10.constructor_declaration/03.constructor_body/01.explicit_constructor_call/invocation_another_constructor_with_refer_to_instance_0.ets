/*---
desc: Invocation constructor of same class with refer to instance variable using 'this' keyword.
assert: An explicit constructor invocation statement in a constructor body may not refer to any instance variables or instance methods or inner classes declared in this class or any superclass, or use this or super in any expression; otherwise, a compile-time error occurs.
tags: [compile-only, negative]
---*/

class A {
  public x: int;
  public y: int;

  public constructor() {
    this(this.y);
  }

  public constructor(a: int) {
    x = a;
  }
}
