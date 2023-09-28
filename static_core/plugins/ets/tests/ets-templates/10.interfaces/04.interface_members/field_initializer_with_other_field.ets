/*---
desc: Field initializer with reference to another interface field.
assert: It is a compile-time error if the initializer of an interface field uses another field whose declaration occurs textually later in the same interface.
tags: [compile-only, negative]
---*/

interface A {
  x: int = y + 5;
  y: int = 5;
}
