/*---
desc: Declarations must be distinguishable in a declaration space.
tags: [compile-only, negative]
---*/

// The constant and the function have the same name:

const PI = 3.14

function PI(): double { return 3.14 }

function main(): int {
  return 0;
}
