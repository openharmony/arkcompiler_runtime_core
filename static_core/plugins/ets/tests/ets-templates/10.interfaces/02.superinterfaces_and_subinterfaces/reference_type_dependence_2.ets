/*---
desc: Declaring subinterface C that extend superclass B with superclass A.
assert: An interface I depends on a reference type T if any of the following is true. I directly depends on an interface J that depends on T (using this definition recursively).
tags: [compile-only]
---*/


class A {
  inner class B {
    interface C {}
  }
}
