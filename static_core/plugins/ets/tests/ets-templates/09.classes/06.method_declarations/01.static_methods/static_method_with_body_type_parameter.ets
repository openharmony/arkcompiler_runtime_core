/*---
desc: Static method declaraion with parameter of type declared in body of method class.
assert: It is a compile-time error to use the name of a parameter of any surrounding declaration in the header or body of a class. 
tags: [compile-only, negative]
---*/

class A {
  inner class B {

  }

  static foo(b: B): void;
}
