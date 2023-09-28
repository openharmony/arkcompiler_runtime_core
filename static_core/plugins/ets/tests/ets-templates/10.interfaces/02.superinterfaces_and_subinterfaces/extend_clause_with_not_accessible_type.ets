/*---
desc: Declaring subinterface.
assert: Each typeReference in the extends clause of an interface declaration must name an accessible interface type (link to access control), or a compile-time error occurs.
tags: [compile-only, negative]
---*/

interface A {
  x: int = 1;
  
}

interface B {
  bar(): int;
}

interface C extends A, B, G {}