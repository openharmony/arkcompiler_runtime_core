/*---
desc: null safety -> Null-Coalescing Operator.
assert: It is a compile-time error if the type of the left-hand-side expression is not reference type.
tags: [negative, compile-only]
---*/

function main(): void {
  let a: int = 5;
  let b: Int = 4;

  let x = a ?? b;    
}
