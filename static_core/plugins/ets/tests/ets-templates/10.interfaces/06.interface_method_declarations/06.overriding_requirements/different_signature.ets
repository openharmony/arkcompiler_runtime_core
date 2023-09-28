/*---
desc: Override public default method from superinterface with different signature.
assert: The relationship between the signature of an interface method and the signatures of any overridden interface methods is specified in link to class’ requirements in overriding and hiding.
tags: [compile-only, negative]
---*/

interface A {
  foo(): int {
    return 1;
  }
}

interface B extends A {
  override foo(c: int): int {
    return 2;
  }
}