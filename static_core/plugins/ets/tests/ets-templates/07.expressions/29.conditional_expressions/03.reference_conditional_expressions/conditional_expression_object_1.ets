/*---
desc: Second and third operands are of type parent/child Object types, bound to A, if A is result
---*/

class A {}
class B extends A {}


function main(): void {
  let a: A = new A();
  let b: B = new B();

  let x = true ? a : b;
  assert x instanceof A;
}