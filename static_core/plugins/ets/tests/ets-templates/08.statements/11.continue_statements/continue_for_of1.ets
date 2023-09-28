/*---
desc: >-
  A continue statement stops execution of the current loop iteration and passes control
  to the next iteration with proper check of the loop exit condition depending on the loop kind.
---*/

function main(): int {
    let sum: int
    let arr: int[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    for (let x of arr) {
        if (x % 2 == 0) {
            continue; // a single loop
        }
        sum += x
    }
    if (sum == 25) return 0;
    return 1;
}
