/*---
desc: Readonly field declaration.
assert: Both class and instance variables (static and non-static fields) may be declared readonly.
tags: [compile-only]
---*/

class A {
  public readonly x: int;

  public constructor() {
    this.x = 1;
  }
}
