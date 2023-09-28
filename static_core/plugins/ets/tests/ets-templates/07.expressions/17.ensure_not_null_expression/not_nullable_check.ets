/*---
desc: null safety -> Not-null operator
assert: A compile-time error occurs if e is not a nullable type.
tags: [negative, compile-only]
---*/

function main(): void {
  let a: Int = 5;
  let b: Int = 4;
  let x = a! + b;

  assert x == 9;
}
