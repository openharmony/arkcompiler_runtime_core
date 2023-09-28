/*---
desc: null safety -> Null-Coalescing Operator.
---*/

class A {}
class BA extends A {}
class CBA extends BA {}
class DA extends A {}

function foo(p: Object): int {
  return 1;
}

function foo(p: A): int {
  return 2;
}

function foo(p: BA): int {
  return 3;
}

function main(): void {
  let a: CBA = new CBA();
  let b: DA = new DA();
  let c = a ?? b;
  assert(foo(c) == 2);

  let d: A = new A();
  let e = a ?? d;
  assert(foo(e) == 2);
  
  let f: BA = new BA();
  let g = a ?? f;
  assert(foo(g) == 3);
}
