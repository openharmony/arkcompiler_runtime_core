/*---
desc: ArithmeticException is thrown by an integer division or integer
  remainder operator if the right-hand operand expression has the zero value.
---*/

function main(): int {
  let a = 1;
  let b = 1;
  let i: long = 0x00000000ffffffff;
  let j: long = 0x0fffffff00000000;

  try {
    let v: String = "X" + (a = 2) + (i as int) / (j as int) / (b = 2);
    return 1;
  } catch (e: ArithmeticException) {
    if (a != 2 || b != 1) return 1;
    return 0;
  } catch (e) {
    return 1;
  }
}
