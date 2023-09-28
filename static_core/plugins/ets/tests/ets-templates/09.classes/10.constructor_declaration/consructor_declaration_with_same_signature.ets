/*---
desc: Class constructors declaration with same signatures.
assert: It is a compile-time error to declare two constructors whose signatures have the same erasure in a class.
tags: [compile-only, negative]
---*/

class A {
  public x: int;

  public constructor() {
    x = 5;
  }

  public constructor() {
    x = 5;
  }
}
