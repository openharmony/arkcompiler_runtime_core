/*---
desc: Invocation constructor of superclass.
assert: The first statement of a constructor body may be an explicit invocation of another constructor of the same class or of the direct superclass.
tags: [compile-only]
---*/

class A {
  public x: int;

  public constructor(a: int) {
    this.x = a;
  }
}

class B extends A {
  public constructor() {
    super(5);
  }
}
