/*---
desc: Field initializer with reference to another interface field.
assert: It is a compile-time error if the keyword this or the keyword super occurs in the initializer of an interface field, unless the occurrence is within the body of an anonymous class.
tags: [compile-only, negative]
---*/

interface A {
  x: int = this.y + 5;
  y: int = 5;
}
