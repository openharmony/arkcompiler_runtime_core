/*---
desc: ArrayIndexOutOfBoundsException is thrown by an array access expression
  if the array index expression has a value that is negative, or greater
  than, or equal to the length of the array.
---*/

function main(): int {
  let v: byte[][][] = new byte[10][][];
  v[0] = new byte[10][];
  let a = 1;
  let b = 1;

  try {
    let i: int = (a = 2) + v[0].length + (v[0][0] = new byte[10]).length + (b = 2);
    if (a != 2 || b != 2) return 1;
    return 0;
  } catch (e: ArrayIndexOutOfBoundsException) {
    return 1;
  } catch {
    return 1;
  }
}
