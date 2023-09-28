/*---
desc: different throws clause in overrided methods.
assert: The relationship between the throws clause of an interface method and the throws clauses of any overridden interface methods is specified in link to class’ requirements in overriding and hiding.
tags: [compile-only, negative]
---*/

interface A {
  m(): int throws {
    return 5;
  }
}

interface B extends A {
  override m(): int {
    return 5;
  }
}