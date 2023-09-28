/*---
desc: Functions must be declared on the top-level.
tags: [compile-only, negative]
---*/

class A {
    function nested(a: int): int {
        return a;
    }
}

function main(): int {
  return 0;
}
