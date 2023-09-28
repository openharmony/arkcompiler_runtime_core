/*---
desc: Declarations must be distinguishable in a declaration space.
tags: [compile-only, negative]
---*/

// Functions have override-equivalent signatures

function foo(x: int) {}

type MyInt = int

function foo(x: MyInt) {}

function main(): int {
  return 0;
}
