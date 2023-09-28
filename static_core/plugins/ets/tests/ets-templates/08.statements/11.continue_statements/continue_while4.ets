/*---
desc: >-
  A continue statement stops execution of the current loop iteration and passes control
  to the next iteration with proper check of the loop exit condition depending on the loop kind.
---*/

function main(): int {
    let sum: int, y: int
    while (y++ < 100) {
        let x = 0
        while (x++ < 100) {
            if (x % 2 == 0) {
                continue; // the inner loop
            }
            sum += x
        }
        if (y % 2 == 0) {
            continue; // the outer loop
        }
    }
    if (sum == 250000) return 0;
    return 1;
}
