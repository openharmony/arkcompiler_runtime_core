/*---
desc: >-
  A continue statement with label identifier transfers control out of the enclosing loop statement
  which has the same label identifier. Such statement must be in the body of the surrounding
  function or method otherwise compile-time error is raised by the compiler.
tags: [negative, compile-only]
---*/

function main(): int {
    let sum: int, y: int
    do {
        let x = 0
        do {
            if (x % 2 == 0) {
                continue loop2; // no such label
            }
            sum += x
        } while (x++ < 100)
    } while (y++ < 3)

    if (sum == 10000) return 0;
    return 1;
}
