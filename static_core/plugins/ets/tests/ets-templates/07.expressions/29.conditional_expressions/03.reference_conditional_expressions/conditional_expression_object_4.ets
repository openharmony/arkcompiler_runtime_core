/*---
desc: Second and third operands children of A
---*/

class A {}
class B extends A {}
class D extends B {}

function main(): void {
  let a: A = new A();
  let d: D = new D();

  let xd = false ? a : d;
  assert x instanceof A;
}