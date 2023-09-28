/*---
desc: Devlaring member with same names.
assert: Fields, methods, and member types of an interface type may not have the same name, as a name must be unique in its declaration space.
tags: [compile-only, negative]
---*/

interface B {
  bar(): int;
  bar: int = 1;
  bar(): void;
}