/*---
desc: Declaring inner class with explicitly static member.
assert: A compile-time error occurs if an inner class declares an explicitly or implicitly static member that is not a constant variable.
tags: [compile-only, negative]
---*/

class A {
  inner class B {
    public static x: int;
  }
}
