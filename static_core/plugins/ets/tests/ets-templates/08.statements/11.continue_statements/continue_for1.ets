/*---
desc: >-
  A continue statement stops execution of the current loop iteration and passes control
  to the next iteration with proper check of the loop exit condition depending on the loop kind.
---*/

function main(): int {
    let sum: int
    for (let x: int = 0; x < 100; x++) {
        if (x % 2 == 0) {
            continue; // a single loop
        }
        sum += x
    }
    if (sum == 2500) return 0;
    return 1;
}
