/*---
desc: Readonly static field declaration.
assert: Both class and instance variables (static and non-static fields) may be declared readonly.
tags: [compile-only]
---*/

class A {
  public static readonly x: int;

  public constructor() {
    A.x = 1;
  }
}
