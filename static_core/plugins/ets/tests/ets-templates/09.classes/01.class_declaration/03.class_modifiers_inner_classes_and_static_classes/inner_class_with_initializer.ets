/*---
desc: Declaring inner class with static initializer.
assert: A compile-time error occurs if an inner class declares a static initializer.
tags: [compile-only, negative]
---*/

class A {
  inner class B {
    static {}
  }
}
