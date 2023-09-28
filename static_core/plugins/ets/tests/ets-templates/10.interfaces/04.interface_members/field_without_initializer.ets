/*---
desc: Field declaration without initializer.
assert: Every declarator in a field declaration of an interface must have an initializer, or a compile-time error occurs.
tags: [compile-only, negative]
---*/

interface A {
  x: int;
}
