/*---
desc: >-
  A continue statement with label identifier transfers control out of the enclosing loop statement
  which has the same label identifier. Such statement must be in the body of the surrounding
  function or method otherwise compile-time error is raised by the compiler.
---*/

function main(): int {
    let sum: int
    let arr1: int[] = [0, 10, 20, 30, 40]
    let arr2: int[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    loop1: for (let y of arr1) {
        loop2: for (let x of arr2) {
            if (x == 7) {
                continue loop1; // the outer loop with label
            }
            if (x % 2 == 0) {
                continue loop2; // the inner loop with label
            }
            sum += x
        }
    }
    if (sum == 45) return 0;
    return 1;
}
