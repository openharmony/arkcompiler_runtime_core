/*---
desc: Declarations must be distinguishable in a declaration space.
tags: [compile-only, negative]
---*/

class A {}

// The type and the variable have the same name:

type Person = A

let Person: Person

function main(): int {
  return 0;
}
