/*---
desc: Extends class with superinterface.
assert: I is a superinterface of the direct superclass of C.
---*/

interface I { }

class A implements I {}

final class B extends A { }

function main(): void {
  let b: I = new B();
}
