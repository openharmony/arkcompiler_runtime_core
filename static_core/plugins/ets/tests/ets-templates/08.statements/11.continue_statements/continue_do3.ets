/*---
desc: >-
  A continue statement stops execution of the current loop iteration and passes control
  to the next iteration with proper check of the loop exit condition depending on the loop kind.
---*/

function main(): int {
    let sum: int, y: int
    do {
        let x = 0
        do {
            sum += x
        } while (x++ < 100)
        if (y % 2 == 0) {
            continue; // the outer loop only
        }
    } while (y++ < 100)

    if (sum == 510050) return 0;
    return 1;
}
