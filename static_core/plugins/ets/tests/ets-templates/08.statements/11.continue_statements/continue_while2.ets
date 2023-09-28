/*---
desc: >-
  A continue statement stops execution of the current loop iteration and passes control
  to the next iteration with proper check of the loop exit condition depending on the loop kind.
---*/

function main(): int {
    let sum: int, y = 0
    while (y++ < 3) {
        let x = 0
        while (x++ < 100) {
            if (x % 2 == 0) {
                continue; // the inner loop only
            }
            sum += x
        }
    }
    if (sum == 7500) return 0;
    return 1;
}
