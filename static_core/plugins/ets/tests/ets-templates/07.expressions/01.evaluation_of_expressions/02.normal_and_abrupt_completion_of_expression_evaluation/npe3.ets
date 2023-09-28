/*---
desc: NullPointerException is thrown by an array access expression
  if the array reference expression has the null value.
---*/

function main(): int {
  let a = 1;
  let b = 1;
  let x: String[]|null = ["A", "B", "C"];

  try {
    let v: String = "" + (x = null) + (a = 2) + x[1] + (b = 2);
    return 1;
  } catch (e: NullPointerException) {
    if (a != 2 || b != 1) return 1;
    return 0;
  } catch {
    return 1;
  }
  return 1;
}
