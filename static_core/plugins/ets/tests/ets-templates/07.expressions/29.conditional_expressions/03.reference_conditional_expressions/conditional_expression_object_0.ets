/*---
desc: Second oprand is value type and third operand are reference type
tags: [negative, compile-only]
---*/

class A {}
class B {}

function main(): void {
  let a: int = 5;
  let b: B = new B();

  let x = true ? a : b;
}