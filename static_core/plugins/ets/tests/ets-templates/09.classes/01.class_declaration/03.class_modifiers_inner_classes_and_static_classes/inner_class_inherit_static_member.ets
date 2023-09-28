/*---
desc: Declaring inner class with inheritance static member.
assert: An inner class can inherit static members, although it cannot declare them, if such are not constant variables.
tags: [compile-only]
---*/

class C {
  public static x: int;
}

class A {
  inner class B extends C {
  }
}
