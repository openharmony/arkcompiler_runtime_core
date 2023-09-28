/*---
desc: Inheritance multiple default method from superinterfaces with same signatures.
assert: If an interface I inherits a default method whose signature is override-equivalent with another method inherited by I, then a compile-time error occurs. (This is the case whether the other method is abstract or default.) Otherwise, all the inherited methods are abstract, and the interface is considered to inherit all the methods.
tags: [compile-only]
---*/

interface A {
  foo(): int;
}

interface B {
  foo(): int;
}

interface C {
  foo(): int;
}

interface D extends A, B, C {

}