/*---
desc: Declaring local class in function body with static member.
assert: A local class can only define or declare non-static members or initializers (similarly to inner class).
tags: [compile-only]
---*/

function bar(): void {
  inner class B {
    public static bar(): void {
    }
  }
}