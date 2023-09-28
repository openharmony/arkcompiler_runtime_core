/*---
desc: Declaring inner class in interface.
assert: The modifier inner pertains to member classes (§9.1.5) only. Member classes that are not declared inner are considered static.
tags: [compile-only]
---*/

interface A {
  inner class B {}
}