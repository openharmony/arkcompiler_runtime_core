/*---
desc: Upper bound class is Object
---*/

class A {}

function foo(p: Object): int {
  return 1;
}

function main(): void {
  let a : A = new A();
  let b : Int = 2;
  let c = true ? a : b;
  assert(foo(c) == 1);
}
