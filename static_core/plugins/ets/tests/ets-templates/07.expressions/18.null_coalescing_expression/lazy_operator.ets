/*---
desc: null safety -> Null-Coalescing Operator.
assert: This operator is lazy, meaning that if the evaluation of the left-hand-side expression is not equal to null, the right-hand-side expression is not evaluated.
---*/

class TestException extends Exception {}

function foo(): int {
  assert false;
  return 0;
}

function main(): void {
  let a: Int = 4;

  let x = a ?? foo();
}
