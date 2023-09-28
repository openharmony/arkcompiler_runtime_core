/*---
desc: Inheritance multiple default method from superinterfaces with same signatures.
assert: If an interface I inherits a default method whose signature is override-equivalent with another method inherited by I, then a compile-time error occurs. (This is the case whether the other method is abstract or default.)
tags: [compile-only, negative]
---*/

interface A {
  foo(): int {
    return 1;
  }
}

interface B {
  foo(): int {
    return 2;
  }
}

interface C {
  foo(): int {
    return 3;
  }
}

interface D extends A, B, C {

}