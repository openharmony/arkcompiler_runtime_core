/*---
desc: >-
  A continue statement with label identifier transfers control out of the enclosing loop statement
  which has the same label identifier. Such statement must be in the body of the surrounding
  function or method otherwise compile-time error is raised by the compiler.
tags: [compile-only, negative]
---*/

function main(): int {
    let sum: int
    let arr1: int[] = [0, 1, 2]
    let arr2: int[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    for (let y of arr1) {
        loop1: for (let x of arr2) {
            sum += x
        }
        if (y % 2 == 0) {
            continue loop1; // misplaced label
        }
    }
    if (sum == 135) return 0;
    return 1;
}
