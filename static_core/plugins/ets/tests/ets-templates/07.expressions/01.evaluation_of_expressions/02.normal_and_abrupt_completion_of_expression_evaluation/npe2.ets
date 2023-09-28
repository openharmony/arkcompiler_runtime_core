/*---
desc: NullPointerException is thrown by an array access expression
  if the array reference expression has the null value.
---*/

function main(): int {
  let a = 1;
  let b = 1;
  let byt: byte;
  let foo = (i: byte): byte[]|null => { return i <= 0 ? null : [i] };

  try {
    let v: String = "X" + (a = 2) + (byt = foo(42)[0]) + (b = 2);
    if (a != 2 || b != 2) return 1;
    return 0;
  } catch (e: NullPointerException) {
    return 1;
  } catch {
    return 1;
  }
  return 1;
}
