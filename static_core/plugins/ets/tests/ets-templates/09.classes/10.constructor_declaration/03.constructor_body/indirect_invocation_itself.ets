/*---
desc: Indirect invocation itself.
assert: It is a compile-time error for a constructor to directly or indirectly invoke itself through a series of one or more explicit constructor invocations involving this.
tags: [compile-only, negative]
---*/

class A {
  public x: int;

  public constructor(a: int) {
    this(5, 5)
  }

  public constructor(a: int, b : int) {
    this(b);
  }
}
