/*---
desc: NullPointerException is thrown by an array access expression
  if the array reference expression has the null value.
---*/

function main(): int {
  let a = 1;
  let b = 1;
  let foo = (i: int): byte[]|null => { return i <= 0 ? null : new byte[i] };

  try {
    let v: String = "X" + (a = 2) + foo(-1)[0] + (b = 2);
    return 1;
  } catch (e: NullPointerException) {
    if (a != 2 || b != 1) return 1;
    return 0;
  } catch {
    return 1;
  }
  return 1;
}
