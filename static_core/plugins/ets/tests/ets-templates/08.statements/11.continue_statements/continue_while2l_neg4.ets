/*---
desc: >-
  A continue statement with label identifier transfers control out of the enclosing loop statement
  which has the same label identifier. Such statement must be in the body of the surrounding
  function or method otherwise compile-time error is raised by the compiler.
tags: [negative, compile-only]
---*/

function main(): int {
    let sum: int, y: int
    loop2: while (y++ < 3) {
        let x = 0
        loop2: while (x++ < 100) {
            if (x % 2 == 0) {
                continue loop2; // duplicate label
            }
            sum += x
        }
    }
    if (sum == 7500) return 0;
    return 1;
}
