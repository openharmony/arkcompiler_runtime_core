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
        do {
            sum += x
        } while (x++ < 100)
        if (y % 2 == 0) {
            continue loop1; // the outer loop only with label
        }
    } while (y++ < 100)

    if (sum == 510050) return 0;
    return 1;
}
