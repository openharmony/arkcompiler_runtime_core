/*---
desc: Check interface with protected access modifier out class.
assert: Member or constructor with protected modifier can be accessed only within their declaring class and classes derived from their declaring class.
tags: [negative, compile-only]
---*/

class A {
  protected interface I { fee(): int }
}

class B implements I {
}
