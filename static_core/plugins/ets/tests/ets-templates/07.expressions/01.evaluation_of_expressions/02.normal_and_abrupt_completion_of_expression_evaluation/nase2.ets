/*---
desc: NegativeArraySizeException is thrown by an array creation expression
  if any dimension expression has a value that is less than zero.
---*/

function main(): int {
  let a: int = 1;
  let b: int = 1;
  let sz: int = 0x0fffffff;

  try {
    let v: int = 42 + (a = 2) + new byte[sz].length + (b = 2);
    if (a != 2 || b != 2) return 1;
    return 0;
  } catch (e: NegativeArraySizeException) {
    return 1;
  } catch (e) {
    return 1;
  }
}
