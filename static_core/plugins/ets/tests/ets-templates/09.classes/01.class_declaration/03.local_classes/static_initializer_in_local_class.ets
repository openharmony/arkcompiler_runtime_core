/*---
desc: Declaring local class in function body with static initializer.
assert: A local class can only define or declare non-static members or initializers (similarly to inner class).
tags: [compile-only, negative]
---*/

function bar(): void {
  inner class B {
    static {}
  }
}