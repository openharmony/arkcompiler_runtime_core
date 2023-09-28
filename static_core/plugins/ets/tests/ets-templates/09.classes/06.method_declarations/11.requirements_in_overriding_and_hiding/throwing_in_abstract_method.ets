/*---
desc: Throws in interface abstract method.
assert: A method that overrides or hides another method, including methods that implement abstract methods defined in interfaces, may not change throw or rethrow clauses of overridden or hidden method.
tags: [compile-only]
---*/

interface A {
  m(): int throws {
    
  }
}

class C implements A {
  public m(): int throws {
    return 5;
  }
}