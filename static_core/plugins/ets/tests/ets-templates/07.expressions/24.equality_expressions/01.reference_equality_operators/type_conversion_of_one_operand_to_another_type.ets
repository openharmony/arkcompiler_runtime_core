/*---
desc: A compile-time error occurs if casting conversion cannot convert the type of either operand to the type of the other.
tags: [compile-only, negative]
---*/

class A {}
class B {}

function main() {
  let a: A = new A();
  let b: B = new B();
  assert a != b;   // CTE expected, A cannot be cast to B
}
