/*---
desc: It is a compile-time error if continueStatement is not within the loopStatement.
tags: [compile-only, negative]
---*/

function main(): int {
    let sum: int
    let arr: int[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    for (let x of arr) {
        sum += x
    }
    if (sum > 0) continue; // misplaced continue statement, out of loop
    return 0;
}
