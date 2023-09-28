/*---
desc: No throws cause in interface abstract method.
assert: A method that overrides or hides another method, including methods that implement abstract methods defined in interfaces, may not change throw or rethrow clauses of overridden or hidden method.
tags: [compile-only, negative]
---*/

interface A {
  m(): int throws;
}

class C implements A {
  public m(): int {
    return 5;
  }
}