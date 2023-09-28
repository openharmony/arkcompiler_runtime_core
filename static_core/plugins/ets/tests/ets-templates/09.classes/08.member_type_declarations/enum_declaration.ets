/*---
desc: Enum declaration in class body.
assert: A member enumeration is an enumeration whose declaration is directly enclosed in the body of another class or interface declaration.
tags: [compile-only, negative]
---*/

class A {
  public enum Color {
    Red, 
    Green
  }
}
