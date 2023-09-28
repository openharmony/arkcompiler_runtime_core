/*---
desc: ArrayIndexOutOfBoundsException is thrown by an array access expression
  if the array index expression has a value that is negative, or greater
  than, or equal to the length of the array.
---*/

function main(): int {
  let v: String[][] = [["A", ""]];
  let a = 1;
  let b = 1;

  try {
    let s: String = "S" + (v[0][1] = "B") + (a = 2) + v[1][0] + (b = 2);
    return 1;
  } catch (e: ArrayIndexOutOfBoundsException) {
    if (a != 2 || b != 1) return 1;
    return 0;
  } catch (e) {
    return 1;
  }
}
