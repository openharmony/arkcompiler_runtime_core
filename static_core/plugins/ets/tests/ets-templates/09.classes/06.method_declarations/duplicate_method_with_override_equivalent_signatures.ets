/*---
desc: Method declaration with existed name and with override-equivalent signatures in class body declaration.
assert: It is a compile-time error for the body of a class declaration to declare as members two methods with the same name and with override-equivalent signatures.
tags: [compile-only, negative]
---*/

class A {
  public run(): void {

  }
}

class B extends A {
  public override run(): void {

  }

  public override run(): void {

  }
}
