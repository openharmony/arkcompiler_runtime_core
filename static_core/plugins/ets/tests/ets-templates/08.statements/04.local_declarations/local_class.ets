/*---
desc: Local declaration in statement (const)
---*/

function main(): void {
  class A {};
  let a: A = new A;
  assert a != null;
}
