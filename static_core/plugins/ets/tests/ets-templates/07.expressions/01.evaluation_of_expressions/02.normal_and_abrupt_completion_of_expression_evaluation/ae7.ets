/*---
desc: ArithmeticException is thrown by an integer division or integer
  remainder operator if the right-hand operand expression has the zero value.
---*/

function main(): int {
  let a = 1;
  let b = 1;
  let i = 1;
  let j: int;

  try {
    let v: boolean = (++a > 0) || (i / j) > 0;
    if (a != 2 || b != 1) return 1;
    return 0;
  } catch (e: ArithmeticException) {
    return 1;
  } catch (e) {
    return 1;
  }
}
