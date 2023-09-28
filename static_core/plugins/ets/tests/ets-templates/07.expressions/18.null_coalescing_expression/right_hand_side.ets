/*---
desc: null safety -> Null-Coalescing Operator.
assert: It checks whether the evaluation of the left-hand-side expression is equal to null and, if it is, the result of null-coalescing expression is the evaluation result of the right-hand-side expression.If the evaluation result of the left-hand-side expression is not equal to null, the result of null-coalescing expression is the evaluation result of the left-hand-side expression.
---*/

function main(): void {
  let a: Int|null = null;
  let b: Int = 4;

  let x: Int = a ?? b;

  assert x == 4;
}
