/*---
desc: NegativeArraySizeException is thrown by an array creation expression
  if any dimension expression has a value that is less than zero.
---*/

function main(): int {
  let sz: int = 0xffffffff;
  let a: int = 1;
  let b: int = 1;
  try {
    let v: String = "" + (a = 2) + new byte[sz].length + (b = 2);
    return 1;
  } catch (e: NegativeArraySizeException) {
    if (a != 2 || b != 1) return 1;
    return 0;
  } catch (e) {
    return 1;
  }
}
