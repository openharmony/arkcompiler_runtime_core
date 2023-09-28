/*---
desc: >-
  A continue statement with label identifier transfers control out of the enclosing loop statement
  which has the same label identifier. Such statement must be in the body of the surrounding
  function or method otherwise compile-time error is raised by the compiler.
---*/

function main(): int {
    let sum: int, y: int
    loop1: do {
        let x = 0
        loop2: do {
            if (x == 10) {
                continue loop1; // the outer loop with label
            }
            if (x % 2 == 0) {
                continue loop2; // the inner loop with label
            }
            sum += x
        } while (x++ < 100)
    } while (y++ < 3)

    if (sum == 100) return 0;
    return 1;
}
