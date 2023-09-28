/*---
desc: Invocation constructor of same class.
assert: The first statement of a constructor body may be an explicit invocation of another constructor of the same class or of the direct superclass.
tags: [compile-only]
---*/

class A {
  public x: int;

  public constructor() {
    this(5);
  }

  public constructor(a: int) {
    this.x = a;
  }
}
