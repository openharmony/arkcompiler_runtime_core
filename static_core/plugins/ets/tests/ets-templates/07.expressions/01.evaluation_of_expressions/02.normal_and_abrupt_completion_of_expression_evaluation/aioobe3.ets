/*---
desc: ArrayIndexOutOfBoundsException is thrown by an array access expression
  if the array index expression has a value that is negative, or greater
  than, or equal to the length of the array.
---*/

function main(): int {
  let v: int[][] = [[1, 2], [3, 4], [5, 6]];
  let a = 1;
  let b = 1;

  try {
    let s: String = "X" + (a = 2) + ++v[1][1] + (b = 2);
    if (a != 2 || b != 2) return 1;
    return 0;
  } catch (e: ArrayIndexOutOfBoundsException) {
    return 1;
  } catch (e) {
    return 1;
  }
}
