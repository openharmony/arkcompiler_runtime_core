/*---
desc: Second and third operands children of A
---*/

class A {}
class B extends A {}
class C extends A {}

function main(): void {
  let c: C = new C();
  let b: B = new B();

  let xd = false ? b : c;
  assert x instanceof A;
}