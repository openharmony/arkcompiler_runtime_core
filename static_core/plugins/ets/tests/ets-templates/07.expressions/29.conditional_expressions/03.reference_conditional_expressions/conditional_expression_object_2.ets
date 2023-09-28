/*---
desc: Second and third operands are of type parent/child Object types, bound to A? if B is result
---*/

class A {}
class B extends A {}


function main(): void {
  let a: A = new A();
  let b: B = new B();

  let xd = false ? a : b;
  assert x instanceof A;
}