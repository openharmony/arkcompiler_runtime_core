/*---
desc: Declaration blank readonly field in inner class.
assert: A blank readonly field of a lexically enclosing type declaration may not be assigned within an inner class, or a compile-time error occurs.
tags: [compile-only, negative]
---*/

class A {
  public inner class B {
    public readonly x: int;
  }
}
