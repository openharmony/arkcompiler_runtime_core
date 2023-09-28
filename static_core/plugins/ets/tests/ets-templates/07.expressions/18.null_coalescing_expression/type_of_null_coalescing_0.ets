/*---
desc: null safety -> Null-Coalescing Operator.
assert: The type of null-coalescing expression is the least upper bound of the non-nullable variant of the type of the left-hand-side expression and the type of the right-hand-side expression.
---*/

class A {}

function foo(p: A): int {
  return 2;
}

function main(): void {
  let a : A = new A();
  let b : A = new A();
  let c = a ?? b;
  assert(foo(c) == 2);
}
