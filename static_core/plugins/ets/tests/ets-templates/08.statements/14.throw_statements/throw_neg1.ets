/*---
desc: >-
  Throw statements cause an exception to be thrown. It is necessary at the compile time
  to check that a throw statement which throws exception is placed in a try block of
  a try statement or inside a throwing function
tags: [compile-only, negative]
---*/

class Ex extends Exception {}
let p: Ex = new Ex();

function main(): int {
    if (1 + 1 == 2) throw p;
    return 0;
}
