/*---
desc: lambda captured local not assigned variable.
assert: Any local variable used but not declared in a lambda body must be assigned before the lambda body, or a compile-time error occurs.
tags: [negative]
---*/

type func = (y: int) => int;

function main(): void {
  let i: int;

  let bar: func = (y: int): int => { return y + i; }

  assert bar(5) == 12;
}