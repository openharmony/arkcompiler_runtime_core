/*---
desc: Method declaration with existed name in class body declaration.
assert: It is a compile-tme error for the body of a class declaration to declare a method with the name that is already used for field or a member type in this declaration.
tags: [compile-only, negative]
---*/

class A {
  private run: int;

  public run(): void {

  }
}
