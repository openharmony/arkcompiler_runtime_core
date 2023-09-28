/*---
desc: Declaring subinterface with direct depence from superclass
assert: An interface I depends on a reference type T if any of the following is true. I directly depends on T.
tags: [compile-only]
---*/

class A {
  x: int;

  interface B{}
}
