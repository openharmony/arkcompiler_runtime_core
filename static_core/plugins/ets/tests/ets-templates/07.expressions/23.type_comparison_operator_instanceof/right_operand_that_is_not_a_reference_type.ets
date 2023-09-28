/*---
desc: Test with an right-operand that is not a reference type
tags: [compile-only, negative]
---*/

function main(): void {
  let num: Int = 10
  assert num instanceof int;
}