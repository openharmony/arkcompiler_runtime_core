/*---
desc: ArithmeticException is thrown by an integer division or integer
  remainder operator if the right-hand operand expression has the zero value.
params: 0%0
---*/

function main(): int {
  let a = 0;
  let b = 1;

  try {
    let v: String = "X" + a++ % (a++ >> 1) % (b = 2);
    return 1;
  } catch (e: ArithmeticException) {
    if (a != 2 || b != 1) return 1;
    return 0;
  } catch (e) {
    return 1;
  }
}
