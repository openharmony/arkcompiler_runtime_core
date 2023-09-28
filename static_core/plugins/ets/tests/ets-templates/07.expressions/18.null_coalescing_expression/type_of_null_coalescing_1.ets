/*---
desc: null safety -> Null-Coalescing Operator.
assert: The type of null-coalescing expression is the least upper bound of the non-nullable variant of the type of the left-hand-side expression and the type of the right-hand-side expression.
---*/

class A {}

function foo(p: Object): int {
  return 1;
}

function main(): void {
  let a : A = new A();
  let b : Int = 2;
  let c = a ?? b;
  assert(foo(c) == 1);
}
