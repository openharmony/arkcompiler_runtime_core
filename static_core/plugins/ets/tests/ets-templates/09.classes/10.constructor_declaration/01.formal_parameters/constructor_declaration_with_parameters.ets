/*---
desc: Class constructors declaration with parameters.
assert: The formal parameters of a constructor are identical in syntax and semantics to those of a method.
tags: [compile-only]
---*/

class A {
  public x: int;

  public constructor(a: int, b: String, c: Object) {
    this.x = 5;
  }

  public constructor(a: boolean, d: float) {
    this.x = 5;
  }
}
