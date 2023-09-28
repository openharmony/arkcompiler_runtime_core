/*---
desc: overriting method with different access method.
assert: The relationship between the accessibility of an interface method and the accessibility of any overridden interface methods is specified in link to class’ requirements in overriding and hiding.
tags: [compile-only, negative]
---*/

interface A {
  private m(): int {
    return 1;
  }
}

interface B extends A {
  override m(): int {
    return 5;
  }
}