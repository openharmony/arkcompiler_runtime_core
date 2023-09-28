/*---
desc: No throws cause in in hided method.
assert: A method that overrides or hides another method, including methods that implement abstract methods defined in interfaces, may not change throw or rethrow clauses of overridden or hidden method.
tags: [compile-only, negative]
---*/

class A {
  public m(): int throws{
    return 5;
  }
}

class C extends A {
  public m(): int {
    return 5;
  }
}
