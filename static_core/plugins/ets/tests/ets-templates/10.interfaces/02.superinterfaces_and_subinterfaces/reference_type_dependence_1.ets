/*---
desc: Declaring subinterface C that extend superinterface B with superclass A.
assert: An interface I depends on a reference type T if any of the following is true. I directly depends on a class C that depends on T.
tags: [compile-only]
---*/


class A {
  interface B {}
}

interface C extends B {}
