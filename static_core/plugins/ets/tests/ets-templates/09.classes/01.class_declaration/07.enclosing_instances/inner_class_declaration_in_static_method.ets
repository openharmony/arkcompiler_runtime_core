/*---
desc: Declaration inner class in static mthod and invoke hin from another method.
assert: An instance of an inner class I whose declaration occurs in a static context has no lexically enclosing instances. However, if I is immediately declared within a static method or static initializer then I does have an enclosing block, which is the innermost block statement exically enclosing the declaration of I.
tags: [compile-only, negative]
---*/

class A {
  public static foo(): void {
    public inner class B {
      public x: int;
    }

    let b: B = new B();
  }

  public static foo(y: int): void {
    let b: B = new B();
  }
}