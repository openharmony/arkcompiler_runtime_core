/*---
desc: >-
  A continue statement with label identifier transfers control out of the enclosing loop statement
  which has the same label identifier. Such statement must be in the body of the surrounding
  function or method otherwise compile-time error is raised by the compiler.
tags: [negative, compile-only]
---*/

function main(): int {
    let sum: int
    let arr1: int[] = [0, 1, 2]
    let arr2: int[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    loop2: for (let y of arr1) {
        loop2: for (let x of arr2) {
            if (x % 2 == 0) {
                continue loop2; // duplicate label
            }
            sum += x
        }
    }
    if (sum == 75) return 0;
    return 1;
}
