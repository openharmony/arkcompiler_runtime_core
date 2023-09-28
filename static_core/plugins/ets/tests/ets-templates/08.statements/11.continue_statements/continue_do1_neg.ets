/*---
desc: >-
  A continue statement stops execution of the current loop iteration and passes control
  to the next iteration with proper check of the loop exit condition depending on the loop kind.
tags: [compile-only, negative]
---*/

function main(): int {
    let sum: int;
    let x = 0;
    do {
        sum += x;
    } while (x++ < 100);
    if (sum > 0) continue; // misplaced continue, out of loop
    return 0;
}
