/*---
desc: Second and third operands is A
---*/

class A {}

function foo(p: A): int {
  return 2;
}

function main(): void {
  let a : A = new A();
  let b : A = new A();
  let c = true ? a : b;
  assert(foo(c) == 2);
}
