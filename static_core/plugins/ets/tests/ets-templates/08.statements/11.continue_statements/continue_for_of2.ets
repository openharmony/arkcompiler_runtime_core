/*---
desc: >-
  A continue statement stops execution of the current loop iteration and passes control
  to the next iteration with proper check of the loop exit condition depending on the loop kind.
---*/

function main(): int {
    let sum: int
    let arr1: int[] = [0, 1, 2]
    let arr2: int[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    for (let y of arr1) {
        for (let x of arr2) {
            if (x % 2 == 0) {
                continue; // the inner loop only
            }
            sum += x
        }
    }
    if (sum == 75) return 0;
    return 1;
}
