/*---
desc: Inherit multiple methods with override-equivalent signatures, but method in superclass is abstract.
assert: It is a compile-time error if a class C inherits a default method whose signature is override-equivalent with another method inherited by C, unless there exists an abstract method declared in a superclass of C and inherited by C that is override-equivalent with the two methods. Otherwise, the set of override-equivalent methods consists of at least one abstract method and zero or more default methods; then the lass is necessarily an abstract class and is considered to inherit all the methods.
tags: [compile-only]
---*/

abstract class A {
  abstract m(): int;
}

interface I {
  m(): int {
    return 1;
  }
}

class C extends A {
  override m(): int {
    return 5;
  }
}
