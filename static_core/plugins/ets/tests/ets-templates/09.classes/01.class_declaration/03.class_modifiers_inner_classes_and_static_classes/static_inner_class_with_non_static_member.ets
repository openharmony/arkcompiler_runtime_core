/*---
desc: Declaring static inner class with non-static member.
assert: A compile-time error occurs if a non-static member of an enclosing class is used within a static class.
tags: [compile-only, negative]
---*/

class A {
  class B {
    public x: int;
  }
}
