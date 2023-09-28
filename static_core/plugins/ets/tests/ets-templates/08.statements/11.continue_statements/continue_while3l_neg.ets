/*---
desc: >-
  A continue statement with label identifier transfers control out of the enclosing loop statement
  which has the same label identifier. Such statement must be in the body of the surrounding
  function or method otherwise compile-time error is raised by the compiler.
tags: [compile-only, negative]
---*/

function main(): int {
    let sum: int, y: int
    while (y++ < 100) {
        let x = 0
        loop1: while (x++ < 100) {
            sum += x
        }
        if (y % 2 == 0) {
            continue loop1; // misplaced label
        }
    }
    if (sum == 505000) return 0;
    return 1;
}
