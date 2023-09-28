/*---
desc: null safety -> Null-Coalescing Operator.
assert: It is a compile-time error if the type of the left-hand-side expression is not reference type.
---*/

function main(): void {
  let a: Int|null = 5;
  let b: int = 4;

  let x = a ?? b;   

  assert x == 5; 
}
