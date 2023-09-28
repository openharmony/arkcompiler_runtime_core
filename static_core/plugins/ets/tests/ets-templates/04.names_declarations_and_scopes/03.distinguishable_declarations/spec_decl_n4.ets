/*---
desc: Declarations must be distinguishable in a declaration space.
tags: [compile-only, negative]
---*/

// Functions have override-equivalent signatures

function foo(x: int) {}

function foo(y: int) {}

function main(): int {
  return 0;
}
