/*---
desc: Test with an left-operand that is not a reference type
tags: [compile-only, negative]
---*/

function main(): void {
  let num: int = 10
  assert num instanceof Int;
}