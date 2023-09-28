/*---
desc: Throws in overrided method.
assert: A method that overrides or hides another method, including methods that implement abstract methods defined in interfaces, may not change throw or rethrow clauses of overridden or hidden method.
tags: [compile-only]
---*/

class A {
  public m(): int throws {
    return 5;
  }
}

final class C extends A {
  public override m(): int throws {
    return 5;
  }
}
