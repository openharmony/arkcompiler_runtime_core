/*---
desc: Override public default method from superinterface with same return type.
assert: The relationship between the return type of an interface method and the return types of any overridden interface methods is specified in link to class’ requirements in overriding and hiding.
tags: [compile-only]
---*/

interface A {
  foo(): int {
    return 1;
  }
}

interface B extends A {
  foo(): int {
    return 2;
  }
}
