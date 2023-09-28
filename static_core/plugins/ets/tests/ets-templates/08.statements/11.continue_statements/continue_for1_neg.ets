/*---
desc: It is a compile-time error if continueStatement is not within the loopStatement.
tags: [compile-only, negative]
---*/

function main(): int {
    let sum: int
    for (let x: int = 0; x < 100; x++) {
        sum += x
    }
    if (sum > 0) continue; // misplaced continue statement, out of loop
    return 0;
}
