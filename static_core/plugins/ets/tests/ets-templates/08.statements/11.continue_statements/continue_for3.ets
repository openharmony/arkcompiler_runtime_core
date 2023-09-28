/*---
desc: >-
  A continue statement stops execution of the current loop iteration and passes control
  to the next iteration with proper check of the loop exit condition depending on the loop kind.
---*/

function main(): int {
    let sum: int
    for (let y: int = 0; y < 100; y++) {
        for (let x: int = 0; x < 100; x++) {
            sum += x
        }
        if (y % 2 == 0) {
            continue; // the outer loop only
        }
    }
    if (sum == 495000) return 0;
    return 1;
}
