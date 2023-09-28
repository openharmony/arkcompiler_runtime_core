/*---
desc: ArithmeticException is thrown by an integer division or integer
  remainder operator if the right-hand operand expression has the zero value.
params: 1/1
---*/

function main(): int {
  let a = 1;
  let b = 1;
  let i = 1;
  let j = 1;

  try {
    let v: String = "X" + (a = 2) + i % j-- + (b = 2);
    if (a != 2 || b != 2) return 1;
    return 0;
  } catch (e: ArithmeticException) {
    return 1;
  } catch (e) {
    return 1;
  }
}
