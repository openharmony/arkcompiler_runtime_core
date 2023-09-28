/*---
desc: Declaring constructor with return statement with expression.
assert: A return statement may be used in the body of a constructor if it does not include an expression. Except for the possibility of explicit constructor invocations, and the prohibition on explicitly returning a value, the body of a constructor is like the body of a method.

tags: [compile-only, negative]
---*/

class A {
  public x: int;

  public constructor() {
    x = 5;
    return this;
  }
}
