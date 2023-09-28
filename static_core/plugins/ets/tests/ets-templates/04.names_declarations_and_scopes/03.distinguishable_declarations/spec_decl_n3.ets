/*---
desc: Declarations must be distinguishable in a declaration space.
tags: [compile-only, negative]
---*/

// The field and the method have the same name.

class C {
    counter: int;

    counter(): int {
      return 0;
    }
}

function main(): int {
  return new C().counter();
}
