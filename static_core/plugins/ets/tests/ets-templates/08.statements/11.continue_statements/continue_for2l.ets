/*---
desc: >-
  A continue statement with label identifier transfers control out of the enclosing loop statement
  which has the same label identifier. Such statement must be in the body of the surrounding
  function or method otherwise compile-time error is raised by the compiler.
---*/

function main(): int {
    let sum: int
    for (let y: int = 0; y < 3; y++) {
        loop2: for (let x: int = 0; x < 100; x++) {
            if (x % 2 == 0) {
                continue loop2; // the inner loop only with label
            }
            sum += x
        }
    }
    if (sum == 7500) return 0;
    return 1;
}
