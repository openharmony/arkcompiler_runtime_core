/*---
desc: Class constructor declaration with throw statement.
assert: The type of a constructor consists of its signature and optional throw or rethrow clauses.
tags: [compile-only]
---*/

class A {
  public x: int;

  constructor(a: int) throws {
    this.x = 5;
  }
}
