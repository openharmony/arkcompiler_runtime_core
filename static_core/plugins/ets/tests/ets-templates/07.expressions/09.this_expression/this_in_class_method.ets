/*---
desc: This in class method.
assert: The keyword this can only be used as an expression in the body of an instance method of a class, enum or interface.
tags: [compile-only]
---*/

class A {
  public x: int;

  public foo(x: int): void { 
    this.x = x;
  }
}