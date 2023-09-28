/*---
desc: Declaring local class in if clause body.
assert: Local classes are defined between balanced braces in a group of zero or more statements (i.e., in a block that is a method body, a for loop or an if clause).
tags: [compile-only]
---*/

class A {
  public bar(): void {
    if (True) {
      inner class B {}
    }
  }
}
